/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:14 by ichpakov          #+#    #+#             */
/*   Updated: 2025/08/08 17:32:20 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/request.hpp"

Request::Request()
{
    
}

Request::Request(int client_fd, const std::string index, const std::string root) : error_code(200)
{
	raw_request = receive_request(client_fd);
	if (raw_request.find("DELETE") != std::string::npos)
		method = "DELETE";
	else if (raw_request.find("POST") != std::string::npos)
    	method = "POST";
    else if (raw_request.find("GET") != std::string::npos)
		method = "GET";
    else
        method = "400";
    path = extract_path(raw_request, index);
    error_check(root);
    if (raw_request.find(".php") != std::string::npos && raw_request.find("favicon.ico") == std::string::npos && error_code == 200)
    {
		cgi = new CGI(*this, root);
	}
	if (cgi)
	{
		if (cgi->getError() != 0)
			error_code = cgi->getError();
	}
}

Request::Request(const Request& copy) : raw_request(copy.raw_request), path(copy.path), method(copy.method), body(copy.body)
{
   
}

Request::~Request()
{
    if (raw_request.find(".php") != std::string::npos)
        delete cgi;
}

//passer conf en parametre pour dossier uploads, body_size, liste methode pour path, modif de path
/*
redirection ex GET /ancienne-page.html HTTP/1.1
HTTP/1.1 301 Moved Permanently
Location: http://example.com/nouvelle-page.html
301 Moved Permanently : la ressource a changé d’adresse définitivement

302 Found (ou Temporary Redirect) : changement temporaire d’adresse

303 See Other : après un POST, redirige vers une autre page (souvent utilisée pour éviter de renvoyer le formulaire)

307 Temporary Redirect : similaire à 302 mais préserve la méthode HTTP (POST reste POST)

308 Permanent Redirect : comme 301 mais préserve la méthode HTTP
*/
void    Request::error_check(std::string root)
{
	std::string fullPath = root + path;
    if (method == "DELETE")
    {
		if (access(fullPath.c_str(), F_OK) != 0)
		{
        	error_code = 404;
		} 
		else
		{
			std::string dirPath = fullPath.substr(0, fullPath.find_last_of('/'));
			if (access(dirPath.c_str(), W_OK | X_OK) != 0)
				error_code = (errno == EACCES) ? 403 : 500;
		}
    }
    else if (method == "POST")
    {
		if (raw_request.find("Content-Length") == std::string::npos)
    	    error_code = 411;
        else if (body.size() > MAX_BODY_SIZE)
            error_code = 413;
		else if (access("uploads/", W_OK | X_OK) != 0)
				error_code = 403;
    }
    else if (method == "GET")
    {
		if (access(fullPath.c_str(), F_OK) != 0)
        	error_code = 404;
		else
		{
			std::string dirPath = fullPath.substr(0, fullPath.find_last_of('/'));
			if (access(dirPath.c_str(), X_OK) != 0) 
				error_code = (errno == EACCES) ? 403 : 500;
			else if (access(fullPath.c_str(), R_OK) != 0)
				error_code = (errno == EACCES) ? 403 : 500;	
		}
    }
	else
	{
		//peut etre faire dans response
		std::string not_allowed[] = {"HEAD", "PUT", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
        for (int i = 0; i < 6; ++i)
        {
            if (raw_request.find(not_allowed[i]) != std::string::npos)
            {
                error_code = 501;
                break;
            }
        }
		if (error_code != 501)
			error_code = 400;
	}

	if (path.size() > MAX_URI_LENGTH)
        error_code = 414;
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

int	Request::get_error_code() const
{
	return (error_code);
}

CGI *Request::get_cgi() const
{
    return (cgi);
}

void	Request::set_error_code(int error)
{
	error_code = error;
}