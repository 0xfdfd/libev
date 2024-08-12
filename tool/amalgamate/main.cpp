#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include "utils/file.hpp"
#include "utils/sha256.hpp"
#include "utils/string.hpp"

typedef struct amalgamate_ctx
{
    amalgamate_ctx()
    {
        noline = false;
    }

    /**
     * @brief Path to input file.
     */
    std::string p_in;

    /**
     * @brief Path to output file.
     */
    std::string p_out;

    /**
     * @brief Commits that apply to the begin of the output file.
     */
    am::StringVec commits;

    /**
     * @brief Disable line control.
     */
    bool    noline;
} amalgamate_ctx_t;

static amalgamate_ctx_t _G;

static const char* s_help =
"amalgamate - Merge source files.\n"
"Parameters:\n"
"  --in=[PATH]\n"
"    Path to input file.\n"
"\n"
"  --out=[PATH]\n"
"    Path to output file.\n"
"\n"
"  --commit=[PATH]\n"
"    Commit file that apply to the output file.\n"
"\n"
"  --noline\n"
"    Disable line control.\n"
;

static void _setup_ctx(int argc, char* argv[])
{
    int i;
    const char* opt;
    size_t opt_sz;

    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("%s\n", s_help);
            exit(EXIT_SUCCESS);
        }

        opt = "--in="; opt_sz = strlen(opt);
        if (strncmp(argv[i], opt, opt_sz) == 0)
        {
            _G.p_in = argv[i] + opt_sz;
            continue;
        }

        opt = "--out="; opt_sz = strlen(opt);
        if (strncmp(argv[i], opt, opt_sz) == 0)
        {
            _G.p_out = argv[i] + opt_sz;
            continue;
        }

        opt = "--commit="; opt_sz = strlen(opt);
        if (strncmp(argv[i], opt, opt_sz) == 0)
        {
            std::string commit = argv[i] + opt_sz;
            _G.commits.push_back(commit);
            continue;
        }

        opt = "-noline";
        if (strcmp(argv[i], opt) == 0)
        {
        	_G.noline = true;
        	continue;
        }

        opt = "--noline="; opt_sz = strlen(opt);
        if (strncmp(argv[i], opt, opt_sz) == 0)
        {
            const char* v = argv[i] + opt_sz;
            if (v[0] == '\0')
            {
                fprintf(stderr, "missing argument to `--noline`.\n");
                exit(EXIT_FAILURE);
            }
            _G.noline = !(strcasecmp(v, "false") == 0 ||
                strcasecmp(v, "0") == 0 ||
                strcasecmp(v, "off") == 0);
            continue;
        }
    }
}

static void _at_exit(void)
{
}

static std::string _generate_commit_for_file(const std::string& path)
{
    std::string out = "/**\n";

    std::string content = am::read_file(path);
    content = am::convert_into_unix(content);

    am::StringVec lines = am::split_lines(content);
    for (am::StringVec::iterator it = lines.begin(); it != lines.end(); it++)
    {
        out += " * " + *it + "\n";
    }
    out += " */\n";

    return out;
}

static std::string _generate_commit(void)
{
    std::string full_commit;
    am::StringVec::iterator it = _G.commits.begin();
    for (; it != _G.commits.end(); it++)
    {
        std::string path = *it;
        std::string commit = _generate_commit_for_file(path);
        full_commit += commit + "\n";
    }
    return full_commit;
}

static bool need_expand(const std::string& line, std::string& path)
{
    if (!am::start_with(line, "#"))
    {
        return false;
    }

    std::string content = line.substr(1);
    content = am::remove_leading_spaces(content);
    if (!am::start_with(content, "include"))
    {
        return false;
    }

    content = content.substr(7);
    content = am::remove_leading_spaces(content);
    if (!am::start_with(content, "\""))
    {
        return false;
    }
    content = content.substr(1);

    size_t pos = content.find('"');
    if (pos == std::string::npos)
    {
        fprintf(stderr, "invalid syntax of `%s`.\n", line.c_str());
        abort();
    }

    path = content.substr(0, pos);

    content = content.substr(pos + 1);
    content = am::remove_leading_spaces(content);
    return !am::start_with(content, "/* @AMALGAMATE: SKIP");
}

static std::string _process_file(const std::string& name, const am::StringVec& lines)
{
    std::string out;
    size_t line_cnt = 1;

    am::StringVec::const_iterator it = lines.begin();
    for (; it != lines.end(); it++, line_cnt++)
    {
        std::string line = *it;

        std::string include_path;
        if (!need_expand(line, include_path))
        {
            out += line + "\n";
            continue;
        }

        std::string content = am::read_file(include_path);
        content = am::convert_into_unix(content);

        out += "////////////////////////////////////////////////////////////////////////////////\n";
        out += "// FILE:    " + include_path + "\n";
        out += "// SIZE:    " + am::to_string(content.size()) + "\n";
        out += "// SHA-256: " + am::sha256(content) + "\n";
        out += "////////////////////////////////////////////////////////////////////////////////\n";
        if (!_G.noline)
        {
        	out += "#line 1 \"" + include_path + "\"\n";
        }
        out += content + "\n";
        if (!_G.noline)
        {
        	out += "#line " + am::to_string(line_cnt + 1) + " \"" + name + "\"\n";
        }
    }
    return out;
}

int main(int argc, char* argv[])
{
    atexit(_at_exit);
    _setup_ctx(argc, argv);

    std::string out = _generate_commit();

    std::string content = am::read_file(_G.p_in);
    content = am::convert_into_unix(content);

    am::StringVec lines = am::split_lines(content);
    out += _process_file(am::file_name(_G.p_in), lines);

    am::write_file(_G.p_out, out);

    return 0;
}
