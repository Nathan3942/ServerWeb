/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 17:53:51 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/01 17:53:51 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once 

#include "server.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>
#include <vector>

class Request;

class CGI
{
    private :
        std::string scriptPath;
        const Request &request;

        //variable pour l'env
        std::string query_string;
        std::string script_name;
        std::string content_type;
        std::string script_filename;

        std::vector<std::string> envStrings;
        std::vector<char*> envp;

        std::string cgiOutput;

        void buildEnv();
        void setupAndRun();
    public :
        CGI(const Request& request, const std::string root);
        //CGI();
        ~CGI();
        void    setup_env_var();
        int execute();
        std::string getOutput() const;
        int getError() const;
};
