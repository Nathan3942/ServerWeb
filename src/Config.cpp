/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/25 11:59:16 by ichpakov          #+#    #+#             */
/*   Updated: 2025/09/25 11:59:16 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Config.hpp"


///////CONSTRUCTORS///////

Config::Config(const char *str) : file_name(str)
{
    parse_config();
	port = collect_all_ports();
}

Config::~Config()
{
}

///////METHODES///////

int Config::parse_config() {
    std::ifstream fd(file_name);
    if (!fd.is_open()) {
        std::cerr << "Error : " << file_name << " can't be open." << std::endl;
        return 0;
    }
    std::string line;
    while (std::getline(fd, line))
    {
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == '#') continue;
        if (line.find("server {") != std::string::npos) {
            ServBlock tmp;
            tmp.parse_ServBlock(fd);
            if (servers.find(tmp.get_name()) == servers.end()) {
                servers[tmp.get_name()] = tmp;
                std::vector<int> ports = tmp.get_port();
                for (std::vector<int>::iterator i = ports.begin(); i != ports.end(); i++)
                {
                    if (port_serverName.find(*i) != port_serverName.end())
                        throw portConflictException();
                    port_serverName[*i] = tmp.get_name();
                }
            }
        }
    }
    fd.close();
    return 1;
}

std::vector<int> Config::collect_all_ports()
{
    std::vector<int> all_ports;

    for (std::map<std::string, ServBlock>::const_iterator it = servers.begin(); it != servers.end(); ++it)
    {
        std::vector<int> ports = it->second.get_port();
        for (std::vector<int>::const_iterator pit = ports.begin(); pit != ports.end(); ++pit)
        {
            all_ports.push_back(*pit);
        }
    }

    return all_ports;
}

ServBlock *Config::get_block_from_port(int port)
{
    std::map<int, std::string>::iterator it = port_serverName.find(port);
	std::string srv_name;
    if (it != port_serverName.end())
		srv_name = it->second;
	else
		return (NULL);
	std::map<std::string, ServBlock>::iterator it2 = servers.find(srv_name);
	if (it2 != servers.end())
		return (&(it2->second));
	else
		return (NULL);
}


