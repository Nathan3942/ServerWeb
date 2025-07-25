/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:48:05 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/24 19:36:52 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//REPONSE HTTP

#pragma once

#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <vector>
#include <fstream>

#include "server.hpp"

#define WEBROOT "./www"

class Request;

class Response
{
private:
	std::ifstream file;
	std::string	header;
	std::string path;
	std::string	content_type;
	bool	header_sent;

	// std::string	content;
	// std::vector<char> http_response;
	
public:
    Response(const std::string& _path, const Request& req, const std::string root, const std::string error);
    ~Response();

	bool	has_more_data() const;
	std::vector<char> get_next_chunk();
	void	close();

	std::string	read_file(const std::string& path);
	std::string get_content_type(const std::string& path);
	std::vector<char> build_reponse(const std::string& body);

	// const std::vector<char>& get_response() const;
};


