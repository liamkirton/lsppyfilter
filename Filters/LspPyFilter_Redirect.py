# ================================================================================
# LspPyFilter
#
# Copyright ©2007 Liam Kirton <liam@int3.ws>
# ================================================================================
# LspPyFilter_Redirect.py
#
# Created: 20/11/2007
# ================================================================================

import lsppyfilter

import re
import socket
import struct
import sys

# ================================================================================

def connect_filter(pid, tid, sid, ip, port):
    new_ip = ip
    new_port = port

    original_ip = "25.254.0.5"
    replacement_ip = "25.0.123.1"

    if socket.inet_aton(original_ip) == struct.pack('!L', ip):
        new_ip = socket.htonl(struct.unpack('L', (socket.inet_aton(replacement_ip)))[0])

        str_ip = socket.inet_ntoa(struct.pack('!L', ip))
        str_new_ip = socket.inet_ntoa(struct.pack('!L', new_ip))
        print '[%x, %x, %x] connect_filter(%s, %d) => (%s, %d)' % (pid, tid, sid, str_ip, port, str_new_ip, new_port)
	
    return (new_ip, new_port)

# ================================================================================

def disconnect_filter(pid, tid, sid):
	pass
	
# ================================================================================

def recv_filter(pid, tid, sid, buffer, length):
	pass

# ================================================================================

def send_filter(pid, tid, sid, buffer, length):
	pass

# ================================================================================
	
if __name__ == '__main__':
	lsppyfilter.set_connect_filter(connect_filter)
	lsppyfilter.set_disconnect_filter(disconnect_filter)
	lsppyfilter.set_recv_filter(recv_filter)
	lsppyfilter.set_send_filter(send_filter)
	
	print '\"Filters\\LspPyFilter_Redirect.py\" Loaded.'

# ================================================================================
