/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:46:56 by ichpakov          #+#    #+#             */
/*   Updated: 2025/08/21 05:25:43 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/response.hpp"

Response::Response(const std::string& _path, Request& req, const std::string root, const std::string error) : path(_path), body_cgi(""), header_sent(false), error_code(200), error_sent(false), autoindex_sent(false)
{
	//voir pour passer path en full_path en argument de la classe pour lavoir dans gnc
	std::string full_path = root + path;
	std::streampos size;
	
	error_msg.clear();
	error_msg[400] = "Bad Request";
	error_msg[403] = "Forbidden";
	error_msg[405] = "Method Not Allowed";
	error_msg[411] = "Length Required";
	error_msg[413] = "Payload Too Large";
	error_msg[414] = "URI Too Long";
	error_msg[500] = "Internal Server Error";
	error_msg[501] = "Not Implemented";
	error_msg[502] = "Bad Gateway";
	error_msg[503] = "Service Unavailable";
	error_msg[504] = "Gateway Timeout";

	if (req.get_cgi())
	{
		if (req.get_error_code() == 200)
			body_cgi = req.get_cgi()->getOutput();
	}


	if (req.get_method() == "POST" && req.get_error_code() == 200)
	{
		time_t now = time(0);
		std::ostringstream oss;
		oss << "upload_" << now << ".txt";
		std::string filename = oss.str();
		std::ofstream ofs((req.get_path_rules()->update_path + filename).c_str(), std::ios::binary);

		if (ofs.is_open() || req.get_path_rules()->upload_enable == true)
		{
			std::string msg = "201 Created";
			if (req.get_body() == "")
				msg = "201 Created";
			if (req.get_cgi())
				ofs << req.get_cgi()->getOutput();
			else
				ofs << req.get_body();
			ofs.close();
			std::ostringstream oss;
			oss << "HTTP/1.1 " << msg << "\r\n";
			oss << "Content-Type: text/html\r\n";
			oss << "Content-Length: 23\r\n";
			oss << "Connection: close\r\n\r\n";
			oss << "<h1>Fichier recu</h1>";
			header = oss.str();
		}
		else
			req.set_error_code(500);
	}
	else if (req.get_method() == "DELETE" && req.get_error_code() == 200)
	{
		if (std::remove(full_path.c_str()) == 0)
			header = "HTTP/1.1 204 No Content\r\n\r\nFile deleted.";
	}
	else if (req.get_method() == "GET" && req.get_error_code() == 200)
	{
		if (req.get_cgi())
		{
			std::ostringstream oss;
			oss << "HTTP/1.1 200 OK\r\n";
			oss << "Content-Type: text/html\r\n";
			oss << "Content-Length: " << req.get_cgi()->getOutput().size() << "\r\n";
			oss << "Connection: close\r\n\r\n";
			header = oss.str();
			std::cout << "CGI output : " << req.get_cgi()->getOutput() << std::endl;
		}
		else
		{
			content_type = get_content_type(path);
			file.open(full_path.c_str(), std::ios::binary);
			if (!file.is_open())
				req.set_error_code(404);
			else
			{
				file.seekg(0, std::ios::end);
				size = file.tellg();
				file.seekg(0, std::ios::beg);
				std::ostringstream oss;
				oss << "HTTP/1.1 200 OK\r\n";
				oss << "Content-Type: " << content_type << "\r\n";
				oss << "Content-Length: " << size << "\r\n";
				oss << "Connection: close\r\n\r\n"; //close keep-alive
				header = oss.str();
			}
		}
	}

	if (req.get_error_code() != 200)
	{
		if (req.get_error_code() == 404)
		{
			std::cout << error << std::endl;
			file.open((root + "/" + error).c_str(), std::ios::binary);
			file.seekg(0, std::ios::end);
			size = file.tellg();
			file.seekg(0, std::ios::beg);
			content_type = get_content_type("/" + error);
			std::ostringstream oss;
			oss << "HTTP/1.1 404 Not Found\r\n";
			oss << "Content-Type: " << content_type << "\r\n";
			oss << "Content-Length: " << size << "\r\n";
			oss << "Connection: close\r\n\r\n"; //close keep-alive
			header = oss.str();
		}
		else if (req.get_error_code() == 1)
		{
			std::string dirPath = root + req.get_path();
			DIR *dir = opendir(dirPath.c_str());
			if (!dir)
			{
				error_code = 500; // fallback sur erreur
			}
			else
			{
				std::ostringstream oss;
				oss << "HTTP/1.1 200 OK\r\n";
				oss << "Content-Type: text/html\r\n";
				// pas de Content-Length mais peut calculer
				oss << "Connection: close\r\n\r\n";
				header = oss.str();

				// on garde la main pour get_next_chunk
				closedir(dir);
			}
		}
		else
		{
			std::cout << "Error code : " << req.get_error_code() << std::endl;
			std::map<int, std::string>::const_iterator it = error_msg.find(req.get_error_code());
			std::ostringstream oss;
			oss << "HTTP/1.1 " << req.get_error_code() << " " << it->second << "\r\n";
			oss << "Content-Type: text/html\r\n";
			oss << "Content-Length: " << 266 + it->second.length() << "\r\n";
			oss << "Connection: close\r\n\r\n";
			header = oss.str();
		}
	}
	error_code = req.get_error_code();
	delete req.get_cgi();
	std::cout << "Header : " << header << std::endl;
}

Response::~Response()
{

}

std::vector<char>	Response::get_next_chunk()
{
	std::vector<char> buffer;
	if (!header_sent)
	{
		header_sent = true;
		buffer.insert(buffer.end(), header.begin(), header.end());
		return (buffer);
	}
	
	if (!error_sent && error_code != 200 && error_code != 404)
	{
		error_sent = true;
		std::map<int, std::string>::const_iterator it = error_msg.find(error_code);
		if (it == error_msg.end())
			return (buffer);
		std::string error_body = generate_error_page(error_code, it->second);
		buffer.insert(buffer.end(), error_body.begin(), error_body.end());
		return (buffer);
	}

	if (!autoindex_sent && error_code == 1)
	{
		autoindex_sent = true;

		std::ostringstream body;
		std::string dirPath = root + path;
		DIR *dir = opendir(dirPath.c_str());
		if (!dir)
			return (buffer); // rien à envoyer

		body << "<!DOCTYPE html>\n<html>\n<head>\n";
		body << "<meta charset=\"UTF-8\">\n";
		body << "<title>Index of " << path << "</title>\n";
		body << "</head>\n<body>\n";
		body << "<h1>Index of " << path << "</h1>\n";
		body << "<ul>\n";

		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name == "." || name == "..")
				continue;

			body << "<li><a href=\"" << path;
			if (path.back() != '/')
				body << "/";
			body << name << "\">" << name << "</a></li>\n";
		}
		body << "</ul>\n</body>\n</html>\n";
		closedir(dir);

		std::string body_str = body.str();
		buffer.insert(buffer.end(), body_str.begin(), body_str.end());
		return (buffer);
	}

	if (body_cgi != "")
	{
		buffer.insert(buffer.end(), body_cgi.begin(), body_cgi.end());
		body_cgi = "";
		return (buffer);
	}

	if (!file.is_open())
		return (buffer);
	
	char	chunk[4096];
	file.read(chunk, sizeof(chunk));
	buffer.insert(buffer.end(), chunk, chunk + file.gcount());
	return (buffer);
}

void	Response::close()
{
	if (file && file.is_open())
		file.close();
}

bool	Response::has_more_data() const
{
	return (file && !file.eof());
}

std::string Response::generate_error_page(int code, const std::string& msg)
{
    std::ostringstream oss;
    oss << "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Error "
        << code << "</title><style>body{font-family:sans-serif;text-align:center;padding-top:100px;background:#f7f7f7;color:#444;}h1{font-size:72px;}p{font-size:24px;}</style></head><body><h1>"
        << code << "</h1><p>" << msg << "</p></body></html>";
    return oss.str();
}

int	Response::get_error_code() const
{
	return (error_code);
}


std::string	Response::get_content_type(const std::string& path)
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


// std::string read_binary(const std::string& filepath)
// {
// 	std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
// 	if (!file)
// 		return ("-404");
	
// 	std::string content;
// 	char	buffer[1024];
// 	while (file.read(buffer, sizeof(buffer)))
// 		content.append(buffer, file.gcount());
// 	if (file.gcount() > 0)
// 		content.append(buffer, file.gcount());
// 	return (content);
// }

// std::string read_default(const std::string& filepath)
// {
//     std::ifstream file(filepath.c_str());
//     if (!file)
//         return ("-404");
//     std::string content;
//     char c;
//     while (file.get(c))
//         content += c;
//     return (content);
// }

// std::string Response::read_file(const std::string& path)
// {
//     std::string full_path = std::string(WEBROOT) + path;
// 	std::cout << full_path << std::endl;
	
// 	if (content_type.find("text/") != std::string::npos)
// 	   	return (read_default(full_path));
// 	else
// 		return (read_binary(full_path));
// }

// std::vector<char> Response::build_reponse(const std::string& body)
// {
// 	std::ostringstream oss;
// 	if (body.find("-404") != std::string::npos)
// 		oss << "HTTP/1.1 404 \r\n";
// 	else
// 		oss << "HTTP/1.1 200 \r\n";
// 	oss << "Content-Type: " << content_type << "\r\n";
// 	oss << "Content-Length: " << body.size() << "\r\n";
// 	// oss << "Content-Disposition: inline\r\n";
// 	oss << "Connection: close\r\n\r\n";

// 	std::string header = oss.str();
// 	std::cout << header << std::endl;
// 	std::vector<char> response;
// 	response.insert(response.end(), header.begin(), header.end());
// 	response.insert(response.end(), body.begin(), body.end());
// 	return response;
// }

// const std::vector<char>& Response::get_response() const
// {
// 	return http_response;
// }

// std::string Response::build_reponse(const std::string& body)
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

// std::string Response::get_response() const
// {
//     return (http_response);
// }
