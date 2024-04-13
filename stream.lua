local WS_URL = "ws://YOUR_URL:9123" -- PUT YOUR URL HERE!!
local args = {...}
local monitor = peripheral.find("monitor")
local x, y = monitor.getSize()
monitor.setTextScale(0.5)

if #args == 0 or args[1] == "--help" or args[1] == "-h" then
    print("Usage: stream <youtube url>. If url is invalid no error will be displayed, otherwise will take a few seconds to load the video.")
    return
end

print("Loading YT url: ", args[1])

local ws = assert(http.websocket(WS_URL))
ws.send(args[1])

while true do
    local data, is_binary = ws.receive(0.05)
    if data ~= nil then
        -- Format:
        -- [2 bytes: palette length in bytes]
        -- [2 bytes: output size of each array]
        -- [2 bytes: width]
        -- [2 bytes: height]
        --      total: 8 bytes

        -- [palette array RGB uint8]
        -- [symbol array]
        -- [fg color palette idx]
        -- [bg color palette idx]
    
        -- local palette_size = data[1] + data[2]
        local palette_size = string.byte(data, 1) + string.byte(data, 2) * 256
        local out_size     = string.byte(data, 3) + string.byte(data, 4) * 256
        local width        = string.byte(data, 5) + string.byte(data, 6) * 256
        local height       = string.byte(data, 7) + string.byte(data, 8) * 256

        local col = 1
        for i = 1, math.floor(palette_size / 3) do
            monitor.setPaletteColour(col,
                string.byte(data, 3 * (i - 1) + 9 + 0) / 255.0,
                string.byte(data, 3 * (i - 1) + 9 + 1) / 255.0,
                string.byte(data, 3 * (i - 1) + 9 + 2) / 255.0
            )
            col = col * 2
        end

        local offset1 = 8 + palette_size
        local offset2 = offset1 + out_size
        local offset3 = offset2 + out_size

        for y = 1, height do
            monitor.setCursorPos(1, y)
            local d1 = (y - 1) * width

            monitor.blit(
                string.sub(data, offset1 + d1, offset1 + d1 + width),
                string.sub(data, offset2 + d1, offset2 + d1 + width),
                string.sub(data, offset3 + d1, offset3 + d1 + width)
            )
        end
    end
end
