/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/01 17:53:14 by ichpakov          #+#    #+#             */
/*   Updated: 2025/10/01 17:53:14 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/CGI.hpp"

// CONSTRUCTORS

namespace {
    void my_usleep(int time)
    {
        struct timeval tv;
        tv.tv_sec = 0;            // secondes
        tv.tv_usec = time * 1000;   // microsecondes = 50 ms
        select(0, NULL, NULL, NULL, &tv);
    }
}

CGI::CGI(const Request& request, const std::string root) : request(request)
{
    setup_env_var();
	script_filename = root + script_name;
	std::cout << "var env set\n";
	std::cout << query_string << " " << script_filename << " " << script_name << std::endl;
}

// CGI::CGI()
// {

// }

CGI::~CGI()
{

}

// PRIVATE

int ft_stoi(std::string s)
{
    int n = 0;
    for (size_t i = 0; i < s.length(); ++i)
    {
        if (isdigit(s[i]))
            n = n * 10 + (s[i] - '0');
        else 
            return 0;
    }
    return n;
}

void CGI::buildEnv()
{
    envStrings.push_back("REQUEST_METHOD=" + request.get_method()); 
    envStrings.push_back("QUERY_STRING=" + query_string); // infos apres le ?
    envStrings.push_back("SCRIPT_FILENAME=" + script_filename); //chemin absolue du fichier cgi - en gros le fichier php - faut le composer
    envStrings.push_back("SCRIPT_NAME=" + script_name); //chemin du script dans l'url
    envStrings.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");

	std::cout << "env cgi :\n";
	for (int i = 0; i < 6; ++i)
		std::cout << envStrings[i] << std::endl;
    if (request.get_method() == "POST")
	{
		std::ostringstream oss;
		oss << request.get_body().size();
        envStrings.push_back("CONTENT_LENGTH=" + oss.str());
        envStrings.push_back("CONTENT_TYPE=" + content_type);
    }

    envStrings.push_back("REDIRECT_STATUS=200");

    for (size_t i = 0; i < envStrings.size(); ++i)
        envp.push_back(const_cast<char*>(envStrings[i].c_str()));
    envp.push_back(NULL);
}

void CGI::setupAndRun()
{
    int stdout_pipe[2];
    int stdin_pipe[2];

    if (pipe(stdout_pipe) < 0)
        return;
    if (pipe(stdin_pipe) < 0)
    {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        return;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        return;
    }

    if (pid == 0)
    {
        // Redirige stdout vers pipe d'écriture
        dup2(stdout_pipe[1], STDOUT_FILENO);
        // Redirige stdin vers pipe de lecture
        dup2(stdin_pipe[0], STDIN_FILENO);

        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);

        chdir(request.get_path().c_str());

        char *argv[] = { const_cast<char*>("/usr/bin/php-cgi"), NULL };
        execve("/usr/bin/php-cgi", argv, envp.data());
    }

    close(stdout_pipe[1]);
    close(stdin_pipe[0]);

    std::string body = request.get_body();
    if (!body.empty())
        write(stdin_pipe[1], body.c_str(), body.size());
    close(stdin_pipe[1]);

    const int TIMEOUT_MS = 5000;    // Timeout global
    const int POLL_STEP_MS = 100;    // Intervalle poll
    int elapsed = 0;

    struct pollfd pfd;
    pfd.fd = stdout_pipe[0];
    pfd.events = POLLIN | POLLHUP;

    bool timed_out = false;
    char buffer[4096];

    while (elapsed < TIMEOUT_MS)
    {
        int ret = poll(&pfd, 1, POLL_STEP_MS);
        if (ret > 0)
        {
            if (pfd.revents & POLLIN)
            {
                ssize_t bytes = read(stdout_pipe[0], buffer, sizeof(buffer));
                if (bytes > 0)
                    cgiOutput.append(buffer, bytes);
                else if (bytes == 0)
                    break;
            }
            else if (pfd.revents & POLLHUP)
            {
                break; // Pipe fermé côté enfant
            }
        }
        else if (ret < 0 && errno != EINTR)
        {
            break; // Erreur poll
        }

        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if (result == pid)
        {
            break;
        }
        elapsed += POLL_STEP_MS;
    }
    
    my_usleep(50); //laisse le temps a l'enfant de _exit() 

    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    if (result == 0)
    {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        timed_out = true;
    }

    close(stdout_pipe[0]);

    if (timed_out)
        cgiOutput = "Status: 504 Gateway Timeout\r\nContent-Type: text/plain\r\n\r\nCGI script timed out.";
}


// PUBLIC METHODES 


int CGI::execute()
{
    buildEnv();
    setupAndRun();
    return 0;
}

std::string CGI::getOutput() const
{
    return cgiOutput;
}

int CGI::getError() const
{
    std::string output(getOutput());
    size_t pos = output.find('\n');
    std::string line = output.substr(0, pos);

    if (line.find("Status:") != std::string::npos)
    {
        std::stringstream ss(line);
        std::string word;
        while (ss >> word)
        {
            if (isdigit(word[0]))
            {
                int nbr = ft_stoi(word);
                return nbr;
            }
        }
    }
    return 0;
}

void CGI::setup_env_var()
{
	std::cout << "setup_env_var\n";
	std::string raw = request.get_raw_request();
	std::cout << "RAW : " << raw << std::endl << std::endl;

	size_t pos_method = raw.find(request.get_method());
	size_t pos_http = raw.find(" HTTP/");
	size_t start_path = pos_method + request.get_method().length() + 1;
	size_t path_length = pos_http - start_path;

	std::string full_path = raw.substr(start_path, path_length);
	std::cout << "Full path: " << full_path << std::endl;

	size_t pos_qs = full_path.find("?");
	if (pos_qs != std::string::npos)
	{
		script_name = full_path.substr(0, pos_qs);
		query_string = full_path.substr(pos_qs + 1);
	}
	else
	{
		script_name = full_path;
		query_string = "";
	}

	std::cout << "Script Name : " << script_name << std::endl;
	std::cout << "Query String : " << query_string << std::endl;

	size_t pos_ct = raw.find("Content-Type: ");
	if (pos_ct != std::string::npos)
		content_type = raw.substr(pos_ct + 14, raw.find("\r\n", pos_ct) - (pos_ct + 14));
	else
		content_type = "";

	std::cout << "Content Type : " << content_type << std::endl;
}
