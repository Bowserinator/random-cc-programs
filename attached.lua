local args = {...}

if args[1] == "--help" or args[1] == "-h" then
    print("Usage: attached. Lists peripherals and updates monitor displays if set.")
    return
end

local scrollBuffer = {}
local pretty = require "cc.pretty"

local _, maxY = term.getSize()
local tx, ty = term.getSize()
local INDENT = pretty.text("  ")

table.insert(scrollBuffer, pretty.text("Attached peripherals:"))
table.insert(scrollBuffer, pretty.text("\16" .. 0 .. ". " .. "term" .. " " .. "(" .. tx .. " x " .. ty .. ")", colors.green))

for i, v in ipairs(peripheral.getNames()) do
    local p = peripheral.wrap(v)
    local ptype = peripheral.getType(v)
    local ptext = pretty.text("\16" .. i .. ". " .. v .. " " .. ptype, colors.green)
    table.insert(scrollBuffer, ptext)

    if (ptype == 'computer') then
        table.insert(scrollBuffer, INDENT .. pretty.text("Is On: ") .. pretty.text(p.isOn() and "[On]" or "[Off]", p.isOn() and colors.green or colors.red))
        table.insert(scrollBuffer, INDENT .. pretty.text("Id   : ") .. pretty.text(p.getId() or "[No id]", colors.blue))
        table.insert(scrollBuffer, INDENT .. pretty.text("Label: ") .. pretty.text(p.getLabel(), colors.blue))
    elseif (ptype == 'drive') then
        local diskPresent = p.isDiskPresent() and pretty.text("[Disk]", colors.green) or pretty.text("[No disk]", colors.red)
        local hasAudio = p.hasAudio() and pretty.text("[Audio]", colors.green) or pretty.text("[No audio]", colors.red)
        local hasData = p.hasData() and pretty.text("[Data]", colors.green) or pretty.text("[No data]", colors.red)

        table.insert(scrollBuffer, INDENT .. diskPresent .. pretty.space .. hasAudio .. pretty.space .. hasData)
        table.insert(scrollBuffer, INDENT .. pretty.text("Label: ") .. pretty.text(p.getDiskLabel() or "None", colors.blue))
        table.insert(scrollBuffer, INDENT .. pretty.text("Mount: ") .. pretty.text(p.getMountPath() or "Not mounted", colors.blue))
        table.insert(scrollBuffer, INDENT .. pretty.text("Audio: ") .. pretty.text(p.getAudioTitle() or "[No audio found]", colors.blue))
    elseif (ptype == 'printer') then
       --  local pageX, pageY = p.getPageSize()
        table.insert(scrollBuffer, INDENT .. pretty.text("Ink level: ") .. pretty.text(p.getInkLevel() .. "", colors.blue))
        table.insert(scrollBuffer, INDENT .. pretty.text("Paper lvl: ") .. pretty.text(p.getPaperLevel() .. "", colors.blue))
    elseif (peripheral.hasType(p, 'inventory')) then
        table.insert(scrollBuffer, INDENT .. pretty.text("Size: ") .. pretty.text(p.size() .. "", colors.blue))
    elseif (peripheral.hasType(p, 'energy_storage')) then
        table.insert(scrollBuffer, INDENT .. pretty.text("Stored: ") .. pretty.text(p.getEnergy() .. " / " .. p.getEnergyCapacity(), colors.blue))
    elseif (ptype == 'monitor') then
        local mx, my = p.getSize()
        table.insert(scrollBuffer,
            INDENT .. pretty.text("Text scale: ") ..
            pretty.text(p.getTextScale() .. "", colors.blue) ..
            pretty.text(", Res: ") ..
            pretty.text(mx .. " x " .. my, colors.blue))
    elseif (ptype == 'command') then
        table.insert(scrollBuffer, INDENT .. pretty.text(p.getCommand() or "No command provided"))
    elseif (ptype == 'modem') then
        local remote = p.getNamesRemote()
        local remote_names = pretty.text("")
        
        table.insert(scrollBuffer, INDENT .. pretty.text("Is Wireless: ") .. pretty.text(p.isWireless() and "[Yes]" or "[No]", p.isWireless() and colors.green or colors.red))
        table.insert(scrollBuffer, INDENT .. pretty.text("Local name : ") .. pretty.text(p.getNameLocal(), colors.blue))
        table.insert(scrollBuffer, INDENT .. pretty.text("Remote names: (total " .. #remote .. ")"))

        if not p.isWireless() then
            for i, v in ipairs(remote) do
                table.insert(scrollBuffer, INDENT .. INDENT .. pretty.text(i .. ": " .. v, colors.blue))
            end
        else
            table.insert(scrollBuffer, INDENT .. INDENT .. pretty.text("[Cannot get names on wireless modem]", colors.red))
        end
    end
end
table.insert(scrollBuffer, pretty.text(""))

function printScroll(y)
    term.clear()
    term.setCursorPos(1, 2)
    for i = 1, maxY - 1 do
        if not scrollBuffer[i + y] then break end
        pretty.print(scrollBuffer[i + y])
    end

    term.setCursorPos(1, maxY)
    local help = "[Any key: quit]"
    term.blit(help .. string.rep(" ", tx - #help), string.rep("f", tx), string.rep("0", tx))
end

local y = 0
printScroll(0)

while true do
    local eventData = {os.pullEvent()}
    local event = eventData[1]

    if event == "mouse_scroll" then
        local direction = eventData[2]
        if direction == 1 then
            if y < #scrollBuffer - maxY then y = y + 1 end
        else
            if y ~= 0 then y = y - 1 end
        end

        printScroll(y)
    elseif event == "key" then
        break
    end
end
