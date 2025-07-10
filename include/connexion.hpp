/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   connexion.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: njeanbou <njeanbou@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/04 17:42:12 by njeanbou          #+#    #+#             */
/*   Updated: 2025/07/07 13:10:36 by njeanbou         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNEXION_HPP
#define CONNEXION_HPP

#include <string>
#include <iostream>
#include <vector>

enum State {READING, WRITING, CLOSED};

class Connexion
{
    private:
        int fd;
        State state;
        std::vector<char> write_buffer;
        size_t  bytes_sent;
    
    public:
		Connexion();
        Connexion(int _fd);

        State get_state() const;
		int	get_fd() const;
        std::vector<char>& get_write_buffer();
        size_t& get_bytes_sent();

        void	set_state(State _state);
        void    set_write_buffer(const std::vector<char>& data);

        void	clear();
};


#endif