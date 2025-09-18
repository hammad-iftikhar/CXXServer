#include <cctype>
#include <fcntl.h>
#include <map>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#include "include/Headers.h"
#include "include/Request.h"
#include "include/cxxserver.h"
#include "include/utils.h"

Request::Request(std::string http_message)
{
    this->http_message = std::move(http_message);
    this->parse();
}

Request::~Request()
{
    if (!body_temp_path.empty())
    {
        unlink(body_temp_path.c_str());
    }
}

void Request::parse()
{
    if (this->http_message.empty())
    {
        return;
    }

    std::vector<std::string> lines = split_string(http_message, "\r\n", true);

    if (lines.empty())
    {
        return;
    }

    std::vector<std::string> first_line_chunks = split_string(lines[0], " ");

    if (first_line_chunks.size() != 3)
    {
        return;
    }

    // Parse first line
    method = to_lowercase(first_line_chunks[0]);
    path = first_line_chunks[1];
    http_version = first_line_chunks[2];

    headers = Headers();

    // Parse headers
    for (int i = 0; i < lines.size(); i++)
    {

        if (i > 0)
        {
            std::string line = lines[i];

            if (line == "")
            {
                break;
            }

            std::pair<std::string, std::string> header = split_once(line, ": ");

            headers.set(header.first, header.second);
        }
    }

    // Parse body

    // Find start of body after headers (first empty line) using CRLFCRLF marker
    size_t headers_body_sep = http_message.find("\r\n\r\n");
    if (headers_body_sep == std::string::npos)
    {
        return;
    }

    body = http_message.substr(headers_body_sep + 4);
    body_size = body.size();

    // Defer detailed body parsing to parse_body()/parse_body_content
    // so that large bodies written to temp files can be handled uniformly.
    // If we already have body bytes here (typical small requests), parse immediately.
    if (!body.empty())
    {
        parse_body_content(body);
    }
}

void Request::parse_body()
{
    if (!body_temp_path.empty())
    {
        // Read the temp file into memory and parse
        std::string content;
        content.reserve(body_size);
        FILE *fp = fopen(body_temp_path.c_str(), "rb");
        if (!fp)
        {
            return;
        }
        const size_t buf_size = 64 * 1024;
        std::vector<char> buf(buf_size);
        while (true)
        {
            size_t n = fread(buf.data(), 1, buf_size, fp);
            if (n == 0)
                break;
            content.append(buf.data(), n);
        }
        fclose(fp);
        parse_body_content(content);
    }
}

void Request::parse_body_content(const std::string &content)
{
    if (body.empty() && !content.empty())
    {
        body = content;
    }

    std::string content_type = headers.get("Content-Type");
    std::string lowered_content_type = to_lowercase(content_type);

    if (lowered_content_type.rfind("application/x-www-form-urlencoded", 0) == 0)
    {
        std::vector<std::string> pairs = split_string(content, "&", true);
        for (const std::string &pair : pairs)
        {
            if (pair.empty())
                continue;
            std::pair<std::string, std::string> kv = split_once(pair, "=");
            std::string key_decoded = url_decode(kv.first);
            std::string value_decoded = url_decode(kv.second);
            form[key_decoded].push_back(value_decoded);
        }
        return;
    }

    if (lowered_content_type.rfind("multipart/form-data", 0) == 0)
    {
        std::string boundary;
        {
            std::vector<std::string> content_type_parts = split_string(content_type, ";");
            for (size_t i = 1; i < content_type_parts.size(); ++i)
            {
                std::pair<std::string, std::string> kv = split_once(content_type_parts[i], "=");
                std::string key = trim(to_lowercase(kv.first));
                std::string val = trim(kv.second);
                if (!val.empty() && val.front() == '"' && val.back() == '"' && val.size() >= 2)
                {
                    val = val.substr(1, val.size() - 2);
                }
                if (key == "boundary")
                {
                    boundary = val;
                    break;
                }
            }
        }

        if (boundary.empty())
        {
            return;
        }

        std::string boundary_marker = "--" + boundary;
        std::string search_next = "\r\n" + boundary_marker;

        size_t pos = 0;
        size_t start_boundary = content.find(boundary_marker, pos);
        if (start_boundary == std::string::npos)
        {
            return;
        }
        pos = start_boundary + boundary_marker.size();

        while (pos < content.size())
        {
            if (content.compare(pos, 2, "--") == 0)
            {
                break;
            }
            if (content.compare(pos, 2, "\r\n") == 0)
            {
                pos += 2;
            }

            size_t part_headers_end = content.find("\r\n\r\n", pos);
            if (part_headers_end == std::string::npos)
            {
                break;
            }
            std::string part_headers_block = content.substr(pos, part_headers_end - pos);
            std::map<std::string, std::string> part_headers_map;
            std::vector<std::string> part_header_lines = split_string(part_headers_block, "\r\n", true);
            for (const std::string &hl : part_header_lines)
            {
                if (hl.empty())
                    continue;
                std::pair<std::string, std::string> kv = split_once(hl, ": ");
                part_headers_map[kv.first] = kv.second;
            }

            size_t content_start = part_headers_end + 4;
            size_t next_boundary_pos = content.find(search_next, content_start);
            bool last_part = false;
            size_t content_end;
            if (next_boundary_pos == std::string::npos)
            {
                size_t closing_pos = content.find("\r\n" + boundary_marker + "--", content_start);
                if (closing_pos != std::string::npos)
                {
                    content_end = closing_pos;
                    last_part = true;
                }
                else
                {
                    content_end = content.size();
                }
            }
            else
            {
                content_end = next_boundary_pos;
            }

            std::string part_content = content.substr(content_start, content_end - content_start);
            if (part_content.size() >= 2 && part_content.compare(part_content.size() - 2, 2, "\r\n") == 0)
            {
                part_content.erase(part_content.size() - 2);
            }

            std::string cd = part_headers_map["Content-Disposition"];
            std::string name_value;
            std::string filename_value;
            if (!cd.empty())
            {
                std::vector<std::string> cd_parts = split_string(cd, ";");
                for (const std::string &p : cd_parts)
                {
                    std::pair<std::string, std::string> kv = split_once(p, "=");
                    std::string k = trim(to_lowercase(kv.first));
                    std::string v = trim(kv.second);
                    if (!v.empty() && v.front() == '"' && v.back() == '"' && v.size() >= 2)
                    {
                        v = v.substr(1, v.size() - 2);
                    }
                    if (k == "name")
                        name_value = v;
                    if (k == "filename")
                        filename_value = v;
                }
            }

            if (!filename_value.empty())
            {
                std::string content_type_part = part_headers_map["Content-Type"];

                std::string tmp_template = TEMP_FILE;
                std::vector<char> tmpl(tmp_template.begin(), tmp_template.end());
                tmpl.push_back('\0');
                int fd = mkstemp(tmpl.data());
                if (fd != -1)
                {
                    ssize_t written = write(fd, part_content.data(), static_cast<size_t>(part_content.size()));
                    (void)written;
                    close(fd);

                    UploadedFile uf;
                    uf.field_name = name_value;
                    uf.filename = filename_value;
                    uf.content_type = content_type_part;
                    uf.temp_path = std::string(tmpl.data());
                    size_t slash = uf.temp_path.find_last_of('/');
                    uf.temp_name = (slash != std::string::npos) ? uf.temp_path.substr(slash + 1) : uf.temp_path;
                    uf.size = part_content.size();
                    files[name_value].push_back(uf);
                }
            }
            else
            {
                form[name_value].push_back(part_content);
            }

            if (last_part)
            {
                break;
            }
            if (next_boundary_pos == std::string::npos)
            {
                break;
            }
            pos = next_boundary_pos + 2 + boundary_marker.size();
        }
        return;
    }
}

Request Request::create_dummy()
{
    return Request("GET / HTTP/1.1\r\n\r\n");
}