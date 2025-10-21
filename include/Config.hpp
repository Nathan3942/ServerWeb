/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ichpakov <ichpakov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:33 by ichpakov          #+#    #+#             */
/*   Updated: 2025/06/20 12:00:34 by ichpakov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServBlock.hpp"

class Config {
    private :
        const char *file_name; //name of ServBlock file
        std::map<std::string, ServBlock> servers; //Server Block object linked to his name
        std::map<int, std::string> port_serverName; //Listen port linked to server name
        std::vector<int> port;

    public :
        class portConflictException : public std::exception {
            virtual const char* what() const throw() {
                return "Conf : try to sett same port for differents servers.";
            }
        };
        Config(const char *str);
        ~Config();

        int	parse_config();
		std::vector<int> collect_all_ports();
    //getters
        std::map<std::string, ServBlock> get_servers() const { return servers; }
        std::map<int, std::string> get_port_serverName() const { return port_serverName; }
        std::vector<int> get_port() const { return port; }
		ServBlock *get_block_from_port(int port);
};
