
local opt = {
    no_line = false
}

local out_path = nil
local out_data = ""

local function split_string (inputstr)
    local sep = ","
    local t={}
    for str in string.gmatch(inputstr, "([^" .. sep .. "]+)") do
        table.insert(t, str)
    end
    return t
end

local function read_files(paths)
    local path_list = split_string(paths)
    for _,v in ipairs(path_list) do
        local f = io.open(v)
        local d = f:read("a")

        d = string.gsub(d, "#%s*include%s+\"([-_%w/]+)%.h\"", "/* AMALGAMATE: %0 */")
        f:close()

        if opt.no_line == false then
            out_data = out_data .. "#line 1 \"" .. v .. "\"\n"
        end
        out_data = out_data .. d .. "\n"
    end
end

local function append_license(path)
    out_data = out_data .. "/**\n"
    local f = io.open(path)
    while true do
        local d = f:read()
        if d == nil then
            break
        end
        out_data = out_data .. " * " .. d .. "\n"
    end
    f:close()
    out_data = out_data .. " */\n"
end

local function append_string(str)
    out_data = out_data .. str .. "\n"
end

for i,v in ipairs(arg) do
    if v == "--no_line" then
        opt.no_line = true
    end

    if v == "--out" then
        out_path = arg[i+1]
    end

    if v == "--license" then
        append_license(arg[i+1])
    end

    if v == "--include" then
        read_files(arg[i+1])
    end

    if v == "--string" then
        append_string(arg[i+1])
    end
end

assert(out_path ~= nil)

-- Write file
do
    local f = io.open(out_path, "w")
    f:write(out_data)
    f:close()
end
