# bitpack

位处理库，用于处理任意位数据。

可以设置任意大小的位数据，设置任意 0/1 值，并能高效查询跟设置。可以把位数据转成字符串存储或者网络传输，同时支持从字符串初始化。

refer to: https://github.com/bcg/bitpack

# use see test.lua
```lua
local bitpack = require "bitpack"

-- 三种创建方式，默认不传就是初始化 32 字节，也就是 256 位数据
-- 传一个数字表示需要创建指定 bytes_num 字节数量大小的位
-- 直接传一个字符串，表示需要用这个字符串初始化需要的位数据
local bp = bitpack.new()
local bp = bitpack.new(bytes_num)
local bp = bitpack.new(str)

bp:size()   -- 返回已使用的位数，比如 bp:on(100)，那么 size 就是 101

bp:alloc_bytes()    -- 返回分配的内存字节

bp:on(index) -- 设置 index 位为 1，位置可以超过当前的 size，超过了就会新分配内存

bp:off(index)    -- 设置 index 位为 0，同上可以超过当前位置

bp:get(index)    -- 获取 index位是 0/1，这个只允许在 size 范围内

bp:set_bytes(index, str)  -- 从 index 位开始设置 bit 为指定字符串 str

bp:get_bytes(index, bytes_num)  -- 获取从 index 位开始的 bytes_num 字节的 bit 值，并转成了字符串返回

bp:append_bytes(str)   -- 从 size 开始往后添加指定字符串 str 的位

bp:to_bytes()   -- 把当前所有 size 内容转成字符串返回

```

