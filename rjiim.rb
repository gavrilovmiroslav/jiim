#!/usr/bin/env ruby

require 'bunny'
require 'socket'

s = UDPSocket.new
s.bind(nil, 5555)

conn = Bunny.new
conn.start

ch = conn.create_channel
x = ch.direct("jaffe")
q = ch.queue("jiim").bind(x)

loop do
	text, sender = s.recvfrom(8192)
	q.publish(text)
end

conn.stop
