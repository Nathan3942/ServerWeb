/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responce.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:48:05 by ichpakov          #+#    #+#             */
/*   Updated: 2025/06/30 13:22:14 by njeanbou         ###   ########.fr       */
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

#include "server.hpp"

#define WEBROOT "./www"

class Responce
{
private:
	std::string	content;
	std::string	content_type;
	std::vector<char> http_response;
	
public:
    Responce(const std::string& path);
    ~Responce();

	std::string	read_file(const std::string& path);
	std::string get_content_type(const std::string& path);
	std::vector<char> build_reponse(const std::string& body);

	const std::vector<char>& get_response() const;
};


