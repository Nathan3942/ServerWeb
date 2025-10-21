/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/25 18:54:59 by njeanbou          #+#    #+#             */
/*   Updated: 2025/10/09 15:15:56 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include <map>
#include <vector>

struct t_location
{
    std::string loc;
    std::string allow_methods;
    int redirCode;
    std::string redirHTTP;
    std::string root;
    std::string upload_store;
    bool directory_listing;
    bool upload_enable;
    bool cgi_extension;
    std::map<int, std::string> error_page;
    std::vector<std::string> index;

    t_location()
        : loc(""),
          allow_methods(""),
          redirCode(0),
          redirHTTP(""),
          root(""),
          upload_store(""),
          directory_listing(false),
          upload_enable(false),
          cgi_extension(false),
          error_page(),
          index()
    {}
};

#endif
