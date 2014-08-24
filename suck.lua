#!/usr/bin/env lua

errno  = require "org.conman.errno"
signal = require "org.conman.signal-ansi"
getopt = require "org.conman.getopt"
net    = require "org.conman.net"

OPTS = 
{
  { "a" , "" , true  , function(a) HOST = a end },
  { "p" , "" , true  , function(p) PORT = tonumber(p) end },
  { "s" , "" , true  , function(s) SCRIPT = s end } ,
  { "h" , "" , false , function()
      io.stderr:write("usage: ",arg[0]," -a addr -p port -s script [-h]\n")
      os.exit(1)
    end
  },
}

if #arg == 0 then
  io.stderr:write("usage: ",arg[0]," -a addr -p port -s script [-h]\n")
  os.exit(1)
end

getopt.getopt(arg,OPTS)

dofile(SCRIPT)

addr = net.address(HOST,'udp',PORT)
sock = net.socket(addr.family,'udp')
sock:bind(addr)
packet_count = 0

signal.catch('int',function()
  print(packet_count)
  print(math.floor(collectgarbage('count')))
  os.exit(0)
end)

while true do
  local remote,data,err = sock:recv()
  if err ~= 0 then
    io.stder:write("sock:recv(): ",errno[err])
  else
    packet_count = packet_count + 1
    main(data)
  end
end
