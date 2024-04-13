local args = {...}

if args[1] == "--help" or args[1] == "-h" then
    print("Usage: symbols. Prints a cool hex grid of symbols.")
    return
end

local scrollBuffer = {}
local pretty = require "cc.pretty"

local _, maxY = term.getSize()
local tx, ty = term.getSize()
local INDENT = pretty.text("  ")

local HEX = "0123456789ABCDEF"
local COLORS = {
    colors.orange, colors.magenta, colors.lightBlue, colors.yellow,
    colors.lime, colors.pink, colors.cyan, colors.purple, colors.blue,
    colors.brown, colors.green, colors.red
}
table.insert(scrollBuffer, pretty.text("  0 1 2 3 4 5 6 7 8 9 A B C D E F", colors.gray))

function mod(a, b)
    return a - (math.floor(a/b)*b)
end

for i = 0, 15 do
    local row = pretty.text(string.sub(HEX, i + 1, i + 1) .. " ", colors.gray)
    for j = 0, 15 do
        local idx = i * 16 + j
        local char = string.char(idx)
        if char == "\n" then char = " " end
        row = row .. pretty.text(char, COLORS[mod(idx, 12) + 1]) .. pretty.text(" ")
    end
    table.insert(scrollBuffer, row)
end
table.insert(scrollBuffer, pretty.text(""))

function printScroll(y)
    term.clear()
    term.setCursorPos(1, 2)
    for i = 1, maxY - 1 do
        if not scrollBuffer[i + y] then break end
        pretty.print(scrollBuffer[i + y])
    end
end

printScroll(0)
