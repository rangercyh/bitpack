local bitpack = require "bitpack"

--[=[
local bp = bitpack.new()
bitpack.new(2400 * 2400)
bitpack.new("asdf")
-- bitpack.new(-199)


print("to_bytes:", bp:to_bytes())
print("size:", bp:size())
print("alloc_bytes:", bp:alloc_bytes())
print("on:", bp:on(10))
print("get:", bp:get(10))
print("off:", bp:off(1))
print("get:", bp:get(10))
print("to_bytes:", bp:to_bytes())
print("off:", bp:off(10))
print("get:", bp:get(10))
print("to_bytes:", bp:to_bytes())
print("get:", bp:get(100))
print("on:", bp:on(100))
print("off:", bp:off(100))
print("size:", bp:size())
print("alloc_bytes:", bp:alloc_bytes())
print("to_bytes:", bp:to_bytes())

print("on:", bp:on(80000))
print("size:", bp:size())
print("alloc_bytes:", bp:alloc_bytes())
print("to_bytes:", bp:to_bytes())


print('=======================')
local bp1 = bitpack.new(4)
print("size:", bp1:size())
print("alloc_bytes:", bp1:alloc_bytes())
print("to_bytes:", bp1:to_bytes())
print("set_bytes:", bp1:set_bytes(0, "asdf"))
print("size:", bp1:size())
print("alloc_bytes:", bp1:alloc_bytes())
local s = bp1:to_bytes()
print("to_bytes:", s, #s, string.byte(s, 1, #s))
s = bp1:get_bytes(0, 4)
print("get_bytes:", s, #s)
print("get_bytes:", bp1:get_bytes(32, 4))
print("get_bytes:", bp1:get_bytes(0, 5))
s = bp1:get_bytes(8, 2)
print("get_bytes:", s, #s)
print("size:", bp1:size())
print("to_bytes:", bp1:to_bytes())
print("append_bytes:", bp1:append_bytes("xyz"))
print("size:", bp1:size())
print("alloc_bytes:", bp1:alloc_bytes())
print("to_bytes:", bp1:to_bytes())
]=]


-- local lz4 = require "lz4"
local bp = require "bitpack"
local zlib = require "zlib"
local zs = zlib.deflate(zlib.BEST_COMPRESSION)

bp = bp.new(3000 * 3000)
bp:on(3000 * 3000 - 1)
function dig(bp, w, h, l)
    local x = math.random(0, w - l - 1)
    local y = math.random(0, h - l - 1)
    local pos = x + y * w
    for i = 0, l - 1 do
        for j = 0, l - 1 do
            bp:on(pos + i * h + j)
        end
    end
end
for i = 1, 1000 do
    dig(bp, 3000, 3000, 100)
end

local s = bp:to_bytes()
print("origin:", #s / 1024)
-- local c = lz4.block_compress_hc(s, 12)
-- print("lz4:", #c / 1024)
local out, eof, bytes_in, bytes_out = zs(s, 'finish')
print('zlib:', bytes_out / 1024)
