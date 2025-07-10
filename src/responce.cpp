/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responce.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:46:56 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/10 16:50:38 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/responce.hpp"

Responce::Responce(const std::string& path)
{
	content_type = get_content_type(path);
    content = read_file(path);
    if (content == "-404")
    {
		content_type = "text/html";
        content = read_file("/404.html");
    }
	std::cout << "Content type : " << content_type << std::endl;
    http_response = build_reponse(content);
}

Responce::~Responce()
{

}

std::string read_binary(const std::string& filepath)
{
	std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
	if (!file)
		return ("-404");
	
	std::string content;
	char	buffer[1024];
	while (file.read(buffer, sizeof(buffer)))
		content.append(buffer, file.gcount());
	if (file.gcount() > 0)
		content.append(buffer, file.gcount());
	return (content);
}

std::string read_default(const std::string& filepath)
{
    std::ifstream file(filepath.c_str());
    if (!file)
        return ("-404");
    std::string content;
    char c;
    while (file.get(c))
        content += c;
    return (content);
}

std::string Responce::read_file(const std::string& path)
{
    std::string full_path = std::string(WEBROOT) + path;
	std::cout << full_path << std::endl;
	
	if (content_type.find("text/") != std::string::npos)
	   	return (read_default(full_path));
	else
		return (read_binary(full_path));
}

std::string	Responce::get_content_type(const std::string& path)
{
	static std::map<std::string, std::string> content_map;
	if (content_map.empty()) {
		content_map["html"] = "text/html";
		content_map["htm"] = "text/html";
		content_map["css"] = "text/css";
		content_map["xml"] = ""; // spécial
		content_map["txt"] = "text/plain";
		content_map["csv"] = "text/csv";
		content_map["js"] = "text/javascript";
		content_map["md"] = "text/markdown";
		content_map["png"] = "image/png";
		content_map["jpg"] = "image/jpeg";
		content_map["jpeg"] = "image/jpeg";
		content_map["gif"] = "image/gif";
		content_map["webp"] = "image/webp";
		content_map["svg"] = "image/svg+xml";
		content_map["bmp"] = "image/bmp";
		content_map["ico"] = "image/x-icon";
		content_map["avif"] = "image/avif";

		content_map["mp3"] = "audio/mpeg";
		content_map["wav"] = "audio/wav";
		content_map["ogg"] = "audio/ogg";
		content_map["aac"] = "audio/aac";
		content_map["webm"] = ""; // traitement spécial

		content_map["mp4"] = "video/mp4";
		content_map["ogv"] = "video/ogg";
		content_map["avi"] = "video/x-msvideo";
		content_map["mpeg"] = "video/mpeg";
		content_map["mpg"] = "video/mpeg";

		content_map["json"] = "application/json";
		content_map["pdf"] = "application/pdf";
		content_map["zip"] = "application/zip";
		content_map["gz"] = "application/gzip";
		content_map["tar"] = "application/x-tar";
		content_map["bin"] = "application/octet-stream";
		content_map["exe"] = "application/octet-stream";
		content_map["doc"] = "application/msword";
		content_map["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		content_map["xls"] = "application/vnd.ms-excel";
		content_map["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		content_map["swf"] = "application/x-shockwave-flash";
		content_map["form"] = "application/x-www-form-urlencoded";
		content_map["eot"] = "application/vnd.ms-fontobject";
		content_map["ttf"] = "application/x-font-ttf";
		content_map["otf"] = "application/x-font-opentype";
		content_map["woff"] = "font/woff";
		content_map["woff2"] = "font/woff2";
	}
	
	size_t dot = path.find_last_of(".");
	if (dot == std::string::npos)
		return ("application/octet-stream");

	std::string afterdot = path.substr(dot + 1);
	
	if (afterdot == "xml") {
        if (path.find("/api/") != std::string::npos || path.find("/soap/") != std::string::npos)
            return "application/xml";
        if (path.find("/docs/") != std::string::npos || path.find("/public/") != std::string::npos)
            return "text/xml; charset=utf-8";
        return "application/xml";
    }

	if (afterdot == "webm") {
		if (path.find("/audio/") != std::string::npos)
			return "audio/webm";
		if (path.find("/video/") != std::string::npos || path.find("/media/") != std::string::npos)
			return "video/webm";
		return "video/webm";
	}
	
	std::map<std::string, std::string>::iterator it = content_map.find(afterdot);
	if (it != content_map.end())
		return (it->second);
	return ("application/octet-stream");
}

std::vector<char> Responce::build_reponse(const std::string& body)
{
	std::ostringstream oss;
	if (body.find("-404") != std::string::npos)
		oss << "HTTP/1.1 404 \r\n";
	else
		oss << "HTTP/1.1 200 \r\n";
	oss << "Content-Type: " << content_type << "\r\n";
	oss << "Content-Length: " << body.size() << "\r\n";
	// oss << "Content-Disposition: inline\r\n";
	oss << "Connection: close\r\n\r\n";

	std::string header = oss.str();
	std::cout << header << std::endl;
	std::vector<char> response;
	response.insert(response.end(), header.begin(), header.end());
	response.insert(response.end(), body.begin(), body.end());
	return response;
}

const std::vector<char>& Responce::get_response() const
{
	return http_response;
}

// std::string Responce::build_reponse(const std::string& body)
// {
//     std::string reponse;
// 	if (body.find("404") != std::string::npos)
// 		reponse += "HTTP/1.1 404 \r\n";
// 	else
// 		reponse += "HTTP/1.1 200 \r\n";
// 	reponse += "Content-Type: " + content_type + "\r\n";
	
// 	char lenght[32];
// 	sprintf(lenght, "%lu", (unsigned long)body.size());
// 	reponse += "Content-Length: " + std::string(lenght) + "\r\n";
// 	reponse += "Connection: close\r\n\r\n";
// 	reponse += body;
// 	return (reponse);
// }

// std::string Responce::get_response() const
// {
//     return (http_response);
// }
