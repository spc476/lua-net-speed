#!/usr/bin/env lua
-- ***************************************************************
--
-- Copyright 2014 by Sean Conner.  All Rights Reserved.
-- 
-- This library is free software; you can redistribute it and/or modify it
-- under the terms of the GNU Lesser General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or (at your
-- option) any later version.
-- 
-- This library is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
-- or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
-- License for more details.
-- 
-- You should have received a copy of the GNU Lesser General Public License
-- along with this library; if not, see <http://www.gnu.org/licenses/>.
--
-- Comments, questions and criticisms can be sent to: sean@conman.org
--
-- ********************************************************************

errno  = require "org.conman.errno"
signal = require "org.conman.signal-ansi"
getopt = require "org.conman.getopt"
net    = require "net"

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

local remote,data,err

while true do
  remote,data,err = sock:recv()
  if err ~= 0 then
    io.stder:write("sock:recv(): ",errno[err])
  else
    packet_count = packet_count + 1
    main(data)
  end
end
