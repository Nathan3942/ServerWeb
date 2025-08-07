/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:14 by ichpakov          #+#    #+#             */
/*   Updated: 2025/08/07 17:59:26 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/request.hpp"

Request::Request()
{
    
}

Request::Request(int client_fd, const std::string index, const std::string root)
{
	raw_request = receive_request(client_fd);
	if (raw_request.find("DELETE") != std::string::npos)
		method = "DELETE";
	else if (raw_request.find("POST") != std::string::npos)
    {
        if (raw_request.find("Content-Length") == std::string::npos)
    	    method = "411";
        else if (body.size() > MAX_BODY_SIZE)
            method = "413";
        else
            method = "POST";
    }
    else if (raw_request.find("GET") != std::string::npos)
		method = "GET";
    else
    {
        method = "400";
        std::string not_allowed[] = {"HEAD", "PUT", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
        for (int i = 0; i < 6; ++i)
        {
            if (raw_request.find(not_allowed[i]) != std::string::npos)
            {
                method = "501";
                break;
            }
        }
    }
    path = extract_path(raw_request, index);
    if (path.size() > MAX_URI_LENGTH)
        method = "414";
    if (raw_request.find(".php") != std::string::npos && raw_request.find("favicon.ico") == std::string::npos)
        cgi = new CGI(*this, root);
    else
        cgi = NULL;
}

Request::Request(const Request& copy) : raw_request(copy.raw_request), path(copy.path), method(copy.method), body(copy.body)
{
    
}

Request::~Request()
{
    if (raw_request.find("/cgi-bin") != std::string::npos)
        delete cgi;
}

std::string Request::receive_request(int client_fd)
{
    const int bufferSize = 8192;
    char buffer[bufferSize];
    std::string request;
    ssize_t bytesRead = 0;

    while (true)
	{
        memset(buffer, 0, bufferSize);
        bytesRead = recv(client_fd, buffer, bufferSize - 1, 0);
        if (bytesRead <= 0)
			break;
        request.append(buffer, bytesRead);
        if (request.find("\r\n\r\n") != std::string::npos)
			break;
    }

    if (request.find("POST") != std::string::npos)
	{
        size_t header_end = request.find("\r\n\r\n");
        if (header_end != std::string::npos)
		{
            size_t cl_pos = request.find("Content-Length:");
            if (cl_pos != std::string::npos)
			{
                size_t eol = request.find("\r\n", cl_pos);
                int content_length = atoi(request.substr(cl_pos + 15, eol - (cl_pos + 15)).c_str());
                size_t available = request.size() - (header_end + 4);

                while (available < (size_t)content_length)
                {
                    memset(buffer, 0, bufferSize);
                    bytesRead = recv(client_fd, buffer, bufferSize - 1, 0);
                    if (bytesRead <= 0)
						break;
                    request.append(buffer, bytesRead);
                    available = request.size() - (header_end + 4);
                }
                body = request.substr(header_end + 4, content_length);
            }
        }
    }

    raw_request = request;
    printf("Requête reçue :\n%s\n", request.c_str());
    return request;
}

std::string	Request::extract_path(const std::string& raw, const std::string index)
{
	std::string path = "/";
	
	size_t pos1 = raw.find(method + " ");
	size_t pos2 = raw.find(" HTTP/");
	if (pos1 != std::string::npos && pos2 != std::string::npos)
		path = raw.substr(pos1 + method.length() + 1, pos2 - (method.length() + 1));
	if (path == "/")
		path = "/" + index;
	return (path);
}

std::string Request::get_body() const
{
	return (body);
}

std::string	Request::get_path() const
{
	return (path);
}

std::string	Request::get_raw_request() const
{
	return (raw_request);
}

std::string Request::get_method() const
{
	return (method);
}

CGI *Request::get_cgi() const
{
    return (cgi);
}