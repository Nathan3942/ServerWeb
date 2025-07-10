/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:14 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/07 12:22:50 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/request.hpp"

Request::Request(int client_fd)
{
	raw_request = receive_request(client_fd);
	path = extract_path(raw_request);
}

Request::~Request()
{

}

std::string	Request::receive_request(int client_fd)
{

    const int bufferSize = 1024;
    char buffer[bufferSize];
    memset(buffer, 0, bufferSize);

    // int bytesRead = read(client_fd, buffer, bufferSize - 1);
	size_t bytesRead = recv(client_fd, buffer, bufferSize - 1, 0);
    if (bytesRead <= 0) {
        perror("read");
        return ("");
    }

    printf("Requête reçue :\n%s\n", buffer);

	// std::string request(buffer);
	return std::string(buffer);
}

std::string	Request::extract_path(const std::string& raw)
{
	std::string path = "/";
	
	size_t pos1 = raw.find("GET ");
	size_t pos2 = raw.find(" HTTP/");
	if (pos1 != std::string::npos && pos2 != std::string::npos)
		path = raw.substr(pos1 + 4, pos2 - 4);
	if (path == "/")
		path = "/index.html";
	return (path);
}

std::string	Request::get_path() const
{
	return (path);
}

std::string	Request::get_raw_request() const
{
	return (raw_request);
}