/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponseBuilder.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ajabri <ajabri@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 11:52:37 by ajabri            #+#    #+#             */
/*   Updated: 2025/07/07 11:53:21 by ajabri           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpResponseBuilder.hpp"

HttpResponseBuilder& HttpResponseBuilder::setVersion(const std::string& version) {
    response.version = version;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setStatus(int code, const std::string& text) {
    response.statusCode = code;
    response.statusText = text;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setHeader(const std::string& key, const std::string& value) {
    response.headers[key] = value;
    return *this;
}

HttpResponseBuilder& HttpResponseBuilder::setBody(const std::string& body) {
    response.body = body;
    return *this;
}

HttpResponse HttpResponseBuilder::build() const {
    return response;
}
