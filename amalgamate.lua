
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

-- Read file and return content
local function read_file(path)
    local file = io.open(path)
    local data = file:read("a")
    file:close()
    return data
end

-- Write content into file
local function write_file(path, content)
    local f = io.open(path, "w")
    f:write(content)
    f:close()
end

local function append_source(paths)
    local path_list = split_string(paths)
    for _,v in ipairs(path_list) do
        local content = read_file(v)
        local pattern = "#%s*include%s+\"([-_%w%./]+)%.h\""
        local replace = "/* AMALGAMATE: %0 */"

        content = string.gsub(content, pattern, replace)

        if opt.no_line == false then
            out_data = out_data .. "#line 1 \"" .. v .. "\"\n"
        end
        out_data = out_data .. content .. "\n"
    end
end

local function append_commit(path)
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

    if v == "--commit" then
        append_commit(arg[i+1])
    end

    if v == "--source" then
        append_source(arg[i+1])
    end

    if v == "--string" then
        append_string(arg[i+1])
    end
end

assert(out_path ~= nil)

-- Write file
write_file(out_path, out_data)
