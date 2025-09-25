/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/20 11:47:14 by ichpakov          #+#    #+#             */
/*   Updated: 2025/09/25 15:38:56 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/request.hpp"

Request::Request()
{
    
}

Request::Request(int client_fd, Config& conf) : cgi(NULL), error_code(200), dir_lst(false)
{
	raw_request = receive_request(client_fd);
	if (raw_request.find("DELETE") != std::string::npos)
		method = "DELETE";
	else if (raw_request.find("POST") != std::string::npos)
    	method = "POST";
    else if (raw_request.find("GET") != std::string::npos)
		method = "GET";
    else
	{
        method = "400";
	}
	s_block = extract_block(conf);
	path = extract_path(raw_request);
	std::cout << "Path extraite de la requete : " << path << std::endl;
	p_rules = extract_location();
	std::cout << "Loca contenu : \nLoc : " << p_rules.loc << "\nAllow method : " << p_rules.allow_methods << "\nRedir http + code " << p_rules.redirHTTP << p_rules.redirCode << "\nRoot : " << p_rules.root << std::endl;
	setup_full_path();
	std::cout << "Path " << path << "\nMethode " << method << std::endl;
    error_check();
    if (raw_request.find(".php") != std::string::npos && raw_request.find("favicon.ico") == std::string::npos && error_code == 200)
    {
		cgi = new CGI(*this, s_block->get_root());
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

ServBlock   *Request::extract_block(Config& conf)
{
	int	port;

	port = extract_port();
	std::cout << "Port extrait : " << port << std::endl;
	return (conf.get_block_from_port(port));
}

int	Request::extract_port()
{
	std::string hostKey = "Host:";
	size_t pos1 = raw_request.find(hostKey);
	if (pos1 == std::string::npos)
		return (80);
	size_t end = raw_request.find("\r\n", pos1);
	std::string hostLine = raw_request.substr(pos1 + hostKey.size(), end - (pos1 + hostKey.size()));

	while (!hostLine.empty() && (hostLine[0] == ' ' || hostLine[0] == '\t'))
		hostLine.erase(0, 1);

	size_t col = hostLine.find(':');
	if (col != std::string::npos)
	{
		std::string portStr = hostLine.substr(col + 1);
		return (std::atoi(portStr.c_str()));
	}
	return (80);
}

t_location	Request::extract_location()
{
	std::string fullPath = s_block->get_root() + path;
	std::map<std::string, t_location> m_rules = s_block->get_locations();
	std::string root = path;
	t_location rules;
	std::cout << "Path " << path << " Fullpath " << fullPath << std::endl;
	while (!root.empty())
	{
		std::map<std::string, t_location>::iterator it = m_rules.find(root);
		if (it != m_rules.end())
		{
			rules = it->second;
			break;
		}
		size_t pos_bs = root.find_last_of('/');
		if (pos_bs == std::string::npos || pos_bs == 0)
		{
			if (pos_bs == 0)
			{
				it = m_rules.find("/");
				if (it != m_rules.end())
				{
					rules = it->second;
					std::cout << "Trouve la loca\n";
				}
			}
			break;
		}
		root = root.substr(0, pos_bs);
	}
	return (rules);
}

void Request::setup_full_path()
{
    if (!p_rules.loc.empty())
    {
		std::cout << "Setup full path avec loca\n";
		if (path == "/")
		{
			std::cout << "Ajout index\n";
			for (size_t i = 0; i < p_rules.index.size(); ++i)
			{
				if (p_rules.index[i] != "")
				{
					path = path + p_rules.index[i];
					std::cout << "Path : " << path << " Index : " << p_rules.index[i] << std::endl;
					break;
				}
			}
		}
        size_t pos_alias = path.find(p_rules.loc);
        if (pos_alias != std::string::npos)
        {
            path.erase(pos_alias, p_rules.loc.size() - 1);
            path.insert(pos_alias, p_rules.root);
        }

		size_t pos_s = 0;
		while ((pos_s = path.find("//", pos_s)) != std::string::npos)
    		path.erase(pos_s, 1);

		// path.insert(0, ".");

		std::cout << "Nouvelle path apres index et alias : " << path << std::endl;
    }
    else
    {
		if (path == "/")
		{
			for (size_t i = 0; i < s_block->get_index().size(); ++i)
			{
				if (s_block->get_index()[i] != "")
				{
					path = path + s_block->get_index()[i];
					break;
				}
			}
		}
        path.insert(0, s_block->get_root());
    }
}

void    Request::error_check()
{
	std::string fullPath = path;
	
	if (!p_rules.loc.empty())
		rules_error(p_rules);
	
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
        else if (body.size() > static_cast<size_t>(s_block->get_client_max_body_size()))
            error_code = 413;
		else if (access(p_rules.upload_store.c_str(), W_OK | X_OK) != 0)
				error_code = 403;
    }
    else if (method == "GET")
    {
		if (access(fullPath.c_str(), F_OK) != 0)
		{
			std::cout << "Error 404 catch\n";
			error_code = 404;
		}
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

void	Request::rules_error(t_location rules)
{
	std::cout << "Rules path " << rules.loc << std::endl;
	if (rules.allow_methods.find(method) == std::string::npos)
	{
		error_code = 405;
		return ;
	}
	if (method == "POST" && rules.upload_enable == false)
		error_code = 500;
	if (method == "GET" && rules.directory_listing == true)
		dir_lst = true;
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

std::string	Request::extract_path(const std::string& raw)
{
	std::string path = "/";
	
	size_t pos1 = raw.find(method + " ");
	size_t pos2 = raw.find(" HTTP/");
	if (pos1 != std::string::npos && pos2 != std::string::npos)
		path = raw.substr(pos1 + method.length() + 1, pos2 - (method.length() + 1));
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

void	Request::set_dir_lst(bool set)
{
	dir_lst = set;
}

t_location	Request::get_path_rules() const
{
	return (p_rules);
}

ServBlock	Request::get_serv_block() const
{
	return (*s_block);
}

bool    Request::get_dir_lst() const
{
	return (dir_lst);
}




// ==12473==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000010 (pc 0x740f6776b898 bp 0x7fffd44cbf90 sp 0x7fffd44cbf68 T0)
// ==12473==The signal is caused by a READ memory access.
// ==12473==Hint: address points to the zero page.
//     #0 0x740f6776b898 in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) (/lib/x86_64-linux-gnu/libstdc++.so.6+0x16b898) (BuildId: ca77dae775ec87540acd7218fa990c40d1c94ab1)
//     #1 0x5e9831f227e3 in ServBlock::get_name[abi:cxx11]() const src/ServBlock.cpp:295
//     #2 0x5e9831efacb5 in Request::Request(int, Config&) src/request.cpp:34
//     #3 0x5e9831f2bcf5 in Server::start() src/server.cpp:227
//     #4 0x5e9831efa51c in main src/main.cpp:44
//     #5 0x740f6722a1c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
//     #6 0x740f6722a28a in __libc_start_main_impl ../csu/libc-start.c:360
//     #7 0x5e9831dce304 in _start (/home/njeanbou/42/wsrv/webserv+0x15304) (BuildId: c5d4985b2682a08c44bc4cd78884f443f9d87f7e)

// AddressSanitizer can not provide additional info.
// SUMMARY: AddressSanitizer: SEGV (/lib/x86_64-linux-gnu/libstdc++.so.6+0x16b898) (BuildId: ca77dae775ec87540acd7218fa990c40d1c94ab1) in std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >::basic_string(std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&)
// ==12473==ABORTING
