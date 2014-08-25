
counts = {}
for i = 0 , 255 do
  counts[i] = 0
end

function main(packet)
  local max = #packet
  for i = 1 , #packet do
    local c = packet:byte(i)
    counts[c] = counts[c] + 1
  end
end
