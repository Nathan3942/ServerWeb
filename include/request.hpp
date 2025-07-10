/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:33 by ichpakov          #+#    #+#             */
/*   Updated: 2025/07/07 12:13:52 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//PARSING REQUETE HTTP

#include <iostream>
#include <string>

#pragma once

#include "server.hpp"

class Request {
    private :
        std::string raw_request;
        std::string path;

    public :
        Request(int client_fd);
        ~Request();
        std::string receive_request(int client_fd);
        std::string extract_path(const std::string& raw);

        std::string get_path() const;
        std::string get_raw_request() const;
};