# ========================================================================================================================
# LspPyFilter
#
# Copyright ©2007 Liam Kirton <liam@int3.ws>
# ========================================================================================================================
# Http_Cookies-Example.py
#
# Created: 26/07/2007
# ========================================================================================================================

import lsppyfilter

import re
import socket
import struct
import sys

# ========================================================================================================================

def connect_filter(pid, tid, sid, ip, port):
	new_ip = ip
	new_port = port

	str_ip = socket.inet_ntoa(struct.pack('!L', ip))
	str_new_ip = socket.inet_ntoa(struct.pack('!L', new_ip))
#	print '[%x, %x, %x] connect_filter(%s, %d) => (%s, %d)' % (pid, tid, sid, str_ip, port, str_new_ip, new_port)
	return (new_ip, new_port)

# ========================================================================================================================

def disconnect_filter(pid, tid, sid):
#	print '[%x, %x, %x] disconnect_filter()' % (pid, tid, sid)
	pass

# ========================================================================================================================

def http_request_filter(pid, tid, sid, request):
	#
	# Print all outgoing "Cookie: " request headers
	#
	cookies_matcher = re.compile('Cookie: (.*)', re.IGNORECASE | re.MULTILINE).search(request)
	if cookies_matcher != None:
		for c in cookies_matcher.groups():
			print '-> Cookie: %s' % c
	
	return request

# ========================================================================================================================
	
def http_response_filter(pid, tid, sid, header, content):
	#
	# Print all incoming "Set-Cookie: " response headers
	#
	set_cookies_matcher = re.compile('Set-Cookie: (.*)', re.IGNORECASE | re.MULTILINE).search(header)
	if set_cookies_matcher != None:
		for c in set_cookies_matcher.groups():
			print '<- Set-Cookie: %s' % c
	
	return (header, content)

# ========================================================================================================================

http_request_matcher = re.compile('^GET (.*) HTTP/1.\d')
http_request_header_matcher = re.compile('(.*)\\r\\n\\r\\n(.*)', re.DOTALL | re.MULTILINE)
http_response_matcher = re.compile('^HTTP/1.\d 200')

http_content_start_matcher = re.compile('.*?\\r\\n\\r\\n', re.DOTALL | re.MULTILINE)
http_content_type_matcher = re.compile('^Content-Type: (.*)', re.IGNORECASE | re.MULTILINE)
http_content_type_html_matcher = re.compile('text/html')
http_content_length_matcher = re.compile('^(Content-Length: )(\d+)', re.IGNORECASE | re.MULTILINE)
http_transfer_encoding_matcher = re.compile('^Transfer-Encoding: (.*)', re.IGNORECASE | re.MULTILINE)
http_transfer_encoding_chunked_matcher = re.compile('chunked')
http_transfer_encoding_chunked_full_matcher = re.compile('^Transfer-Encoding: chunked\r\n', re.IGNORECASE | re.MULTILINE)
http_transfer_encoding_chunked_chunk_matcher = re.compile('^([0-9a-fA-F]*)[ \t\f\v]*(\r\n)?')
http_accept_encoding_matcher = re.compile('^Accept-Encoding: gzip,\s*deflate\r\n', re.IGNORECASE | re.MULTILINE)

# ========================================================================================================================

def recv_filter(pid, tid, sid, buffer, length):
	# HTTP/1.1 200 response
	if http_response_matcher.match(buffer) != None:
		
		# Separate HTTP header from content
		content_start_match = http_content_start_matcher.match(buffer)
		if content_start_match != None:
			content_start = content_start_match.end(0)
			
			# Content-Type specified
			content_type_match = http_content_type_matcher.search(buffer)
			if content_type_match != None:
				content_type = content_type_match.groups()[0]
				
				# Content-Type: text/html
				if http_content_type_html_matcher.search(content_type) != None:
					content_length_match = http_content_length_matcher.search(buffer)
					transfer_encoding_match = http_transfer_encoding_matcher.search(buffer)
					
					# Content-Length specified
					if content_length_match != None:
						content_length = int(content_length_match.groups()[1])
						recv_remaining = content_length - (length - content_start)
						if recv_remaining > 0:
							return recv_remaining
							
						else:
							filtered_header, filtered_content = http_response_filter(pid, tid, sid, buffer[0:content_start-4], buffer[content_start:content_start + content_length])
							filtered_header = http_content_length_matcher.sub('\g<1>' + str(len(filtered_content)), filtered_header)
							buffer = filtered_header + '\r\n\r\n' + filtered_content
					
					# Transfer-Encoding specified
					elif transfer_encoding_match != None:
						transfer_encoding = transfer_encoding_match.groups()[0]
						
						# Transfer-Encoding: chunked
						if http_transfer_encoding_chunked_matcher.search(transfer_encoding) != None:
							header_buffer = buffer[0:content_start-4]
							content_buffer = buffer[content_start:length]
							content_buffer_length = length - content_start
							content_buffer_current_chunk_length = -1
							
							de_chunked_content_buffer = ''
							
							# Loop to extract data into de_chunked_content_buffer, until either we run out
							# or hit a zero-length chunk
							i = 0
							while i < content_buffer_length:
								content_buffer_current_chunk_length_match = http_transfer_encoding_chunked_chunk_matcher.match(content_buffer[i:content_buffer_length])
								if content_buffer_current_chunk_length_match != None:
									content_buffer_current_chunk_length = int(content_buffer_current_chunk_length_match.groups()[0], 16)
									i += content_buffer_current_chunk_length_match.end(0)
									
									# Positively sized chunk
									if content_buffer_current_chunk_length > 0:
										de_chunked_content_buffer = de_chunked_content_buffer + content_buffer[i:i+content_buffer_current_chunk_length]
										i += content_buffer_current_chunk_length + 2
										
										# We don't have the full chunk, return a value indicating the number of bytes required
										if i >= content_buffer_length:
											require_bytes = i - content_buffer_length
											if require_bytes <= 0:
												require_bytes = 1
											return require_bytes
									
									# Zero sized chunk
									else:
										break
								
								# Error parsing chunked content
								else:
									break
							
							# If the current chunk isn't zero sized, we haven't received the full response
							if content_buffer_current_chunk_length != 0:
								return 1
							
							# We have the full response, pretend that it was passed with a Content-Length
							header_buffer = http_transfer_encoding_chunked_full_matcher.sub('Content-Length: 1\r\n', header_buffer)
							content_buffer = de_chunked_content_buffer
							
							# Filter content
							filtered_header, filtered_content = http_response_filter(pid, tid, sid, header_buffer, content_buffer)
							
							# Corrent Content-Length header
							filtered_header = http_content_length_matcher.sub('\g<1>' + str(len(filtered_content)), filtered_header)
							buffer = filtered_header + '\r\n\r\n' + filtered_content
	
	return buffer

# ========================================================================================================================

def send_filter(pid, tid, sid, buffer, length):
	# HTTP GET header
	if http_request_matcher.match(buffer) != None:
		request_groups = http_request_header_matcher.search(buffer).groups()
		
		# Remove Accept-Encoding header, to ensure that we receive an uncompressed response
		filtered_buffer = http_accept_encoding_matcher.sub('', request_groups[0])
		filtered_buffer = http_request_filter(pid, tid, sid, filtered_buffer)
		if filtered_buffer != None:
			buffer = filtered_buffer + '\r\n\r\n' + request_groups[1]

	return buffer

# ========================================================================================================================

if __name__ == '__main__':
	lsppyfilter.set_connect_filter(connect_filter)
	lsppyfilter.set_disconnect_filter(disconnect_filter)
	lsppyfilter.set_recv_filter(recv_filter)
	lsppyfilter.set_send_filter(send_filter)
	
	print '\"Filters\\Http_Cookies-Example.py\" Loaded.'

# ========================================================================================================================
# ========================================================================================================================
