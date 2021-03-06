#include <dirent.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>

#include "pal/FWThread.h"

#define SERVER_NAME "Web++"
#define SERVER_VERSION "1.0.1"

#define BUFSIZE 8096

using namespace std;

namespace WPP {
    class Request {
        public:
            Request() {

            }
            std::string method;
            std::string path;
            std::string params;
            map<string, string> headers;
            map<string, string> query;
            map<string, string> cookies;

        private:

    };

    class Response {
        public:
            Response() {
                code = 200;
                phrase = "OK";
                type = "text/html";
                body << "";
                
                // set current date and time for "Date: " header
                char buffer[100];
                time_t now = time(0);
                struct tm tstruct;
                tstruct = *gmtime_r(&now, &tstruct);
                strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", &tstruct);
                date = buffer;
            }
            int code;
            string phrase;
            string type;
            string date;
            stringstream body;
            map<string, string> extra_headers;
        
            void send(string str) {
               body << str;
            };
            void send(const char* str) {
               body << str;
            };
        private:
    };

    typedef std::function<void(Request *, Response *)> Callback;
    
    class Exception : public std::exception {
        public:
            Exception() : pMessage("") {}
            Exception(const char* pStr) : pMessage(pStr) {}
            const char* what() const throw () { return pMessage; }
        private:
            const char* pMessage;
    //        const int pCode;
    };

    map<string, string> mime;

    void list_dir(Request* req, Response* res) {
        unsigned char isFile = 0x8, isFolder = 0x4;
        struct dirent *dir;
        int status;
        struct stat st_buf;

        // Mime
        mime["atom"] = "application/atom+xml";
        mime["hqx"] = "application/mac-binhex40";
        mime["cpt"] = "application/mac-compactpro";
        mime["mathml"] = "application/mathml+xml";
        mime["doc"] = "application/msword";
        mime["bin"] = "application/octet-stream";
        mime["dms"] = "application/octet-stream";
        mime["lha"] = "application/octet-stream";
        mime["lzh"] = "application/octet-stream";
        mime["exe"] = "application/octet-stream";
        mime["class"] = "application/octet-stream";
        mime["so"] = "application/octet-stream";
        mime["dll"] = "application/octet-stream";
        mime["dmg"] = "application/octet-stream";
        mime["oda"] = "application/oda";
        mime["ogg"] = "application/ogg";
        mime["pdf"] = "application/pdf";
        mime["ai"] = "application/postscript";
        mime["eps"] = "application/postscript";
        mime["ps"] = "application/postscript";
        mime["rdf"] = "application/rdf+xml";
        mime["smi"] = "application/smil";
        mime["smil"] = "application/smil";
        mime["gram"] = "application/srgs";
        mime["grxml"] = "application/srgs+xml";
        mime["air"] = "application/vnd.adobe.apollo-application-installer-package+zip";
        mime["mif"] = "application/vnd.mif";
        mime["xul"] = "application/vnd.mozilla.xul+xml";
        mime["xls"] = "application/vnd.ms-excel";
        mime["ppt"] = "application/vnd.ms-powerpoint";
        mime["rm"] = "application/vnd.rn-realmedia";
        mime["wbxml"] = "application/vnd.wap.wbxml";
        mime["wmlc"] = "application/vnd.wap.wmlc";
        mime["wmlsc"] = "application/vnd.wap.wmlscriptc";
        mime["vxml"] = "application/voicexml+xml";
        mime["bcpio"] = "application/x-bcpio";
        mime["vcd"] = "application/x-cdlink";
        mime["pgn"] = "application/x-chess-pgn";
        mime["cpio"] = "application/x-cpio";
        mime["csh"] = "application/x-csh";
        mime["dcr"] = "application/x-director";
        mime["dir"] = "application/x-director";
        mime["dxr"] = "application/x-director";
        mime["dvi"] = "application/x-dvi";
        mime["spl"] = "application/x-futuresplash";
        mime["gtar"] = "application/x-gtar";
        mime["hdf"] = "application/x-hdf";
        mime["js"] = "application/x-javascript";
        mime["latex"] = "application/x-latex";
        mime["sh"] = "application/x-sh";
        mime["shar"] = "application/x-shar";
        mime["swf"] = "application/x-shockwave-flash";
        mime["sit"] = "application/x-stuffit";
        mime["sv4cpio"] = "application/x-sv4cpio";
        mime["sv4crc"] = "application/x-sv4crc";
        mime["tar"] = "application/x-tar";
        mime["tcl"] = "application/x-tcl";
        mime["tex"] = "application/x-tex";
        mime["man"] = "application/x-troff-man";
        mime["me"] = "application/x-troff-me";
        mime["ms"] = "application/x-troff-ms";
        mime["xml"] = "application/xml";
        mime["xsl"] = "application/xml";
        mime["xhtml"] = "application/xhtml+xml";
        mime["xht"] = "application/xhtml+xml";
        mime["dtd"] = "application/xml-dtd";
        mime["xslt"] = "application/xslt+xml";
        mime["zip"] = "application/zip";
        mime["mp3"] = "audio/mpeg";
        mime["mpga"] = "audio/mpeg";
        mime["mp2"] = "audio/mpeg";
        mime["m3u"] = "audio/x-mpegurl";
        mime["wav"] = "audio/x-wav";
        mime["pdb"] = "chemical/x-pdb";
        mime["xyz"] = "chemical/x-xyz";
        mime["bmp"] = "image/bmp";
        mime["cgm"] = "image/cgm";
        mime["gif"] = "image/gif";
        mime["ief"] = "image/ief";
        mime["jpg"] = "image/jpeg";
        mime["jpeg"] = "image/jpeg";
        mime["jpe"] = "image/jpeg";
        mime["png"] = "image/png";
        mime["svg"] = "image/svg+xml";
        mime["wbmp"] = "image/vnd.wap.wbmp";
        mime["ras"] = "image/x-cmu-raster";
        mime["ico"] = "image/x-icon";
        mime["pnm"] = "image/x-portable-anymap";
        mime["pbm"] = "image/x-portable-bitmap";
        mime["pgm"] = "image/x-portable-graymap";
        mime["ppm"] = "image/x-portable-pixmap";
        mime["rgb"] = "image/x-rgb";
        mime["xbm"] = "image/x-xbitmap";
        mime["xpm"] = "image/x-xpixmap";
        mime["xwd"] = "image/x-xwindowdump";
        mime["css"] = "text/css";
        mime["html"] = "text/html";
        mime["htm"] = "text/html";
        mime["txt"] = "text/plain";
        mime["asc"] = "text/plain";
        mime["rtx"] = "text/richtext";
        mime["rtf"] = "text/rtf";
        mime["tsv"] = "text/tab-separated-values";
        mime["wml"] = "text/vnd.wap.wml";
        mime["wmls"] = "text/vnd.wap.wmlscript";
        mime["etx"] = "text/x-setext";
        mime["mpg"] = "video/mpeg";
        mime["mpeg"] = "video/mpeg";
        mime["mpe"] = "video/mpeg";
        mime["flv"] = "video/x-flv";
        mime["avi"] = "video/x-msvideo";
        mime["movie"] = "video/x-sgi-movie";

        char* actual_path;
        char* base_path = realpath(req->params.c_str(), NULL);
        string new_path = "";
        actual_path = realpath(req->params.c_str(), NULL);

        if(req->query.find("open") != req->query.end()) {
            new_path += req->query["open"];
            strcat(actual_path, new_path.c_str());
        }

        // prevent directory traversal
        char* effective_path = realpath(actual_path, NULL);
        if ((effective_path != NULL) && (strncmp(base_path, effective_path, strlen(base_path)) != 0)) {
            free(actual_path);
            actual_path = base_path;
            new_path = "";
        }
        free(effective_path);
        effective_path = NULL;

        status = stat(actual_path, &st_buf);

        if (status != 0)  {
            res->code = 404;
            res->phrase = "Not Found";
            res->type = "text/plain";
            res->send("Not found");
        } else if (S_ISREG (st_buf.st_mode)) {
            size_t ext_pos = string(actual_path).find_last_of(".");

            map<string, string>::iterator ext = mime.find(string(actual_path).substr(ext_pos + 1));

            if(ext != mime.end()) {
                res->type = ext->second;
            } else {
                res->type = "application/octet-stream";
            }

            ifstream ifs(actual_path);

            copy(istreambuf_iterator<char>(ifs),
                 istreambuf_iterator<char>(),
                 ostreambuf_iterator<char>(res->body));
        } else if (S_ISDIR (st_buf.st_mode)) {
            DIR* dir_d = opendir(actual_path);

            if (dir_d == NULL) throw WPP::Exception("Unable to open / folder");

            std::stringstream out;
            out << "<title>" << new_path << "</title>" << endl;
            out << "<table>";

            while((dir = readdir(dir_d))) {
                out << "<tr><td><a href=\"" << req->path << "?open=" << new_path << "/" << dir->d_name << """\">";

                if (dir->d_type == isFolder) {
                    out << "[" << dir->d_name << "]";
                } else {
                    out << " " << dir->d_name << "";
                }

                out << "</a></td></tr>";
            }

            out << "</table>";

            res->send(out.str().c_str());
        }
        
        if (actual_path != base_path) {
            free(actual_path);
        }
        free(base_path);
    }

    struct Route {
        regex path;
        string method;
        Callback callback;
        string params;
    };

    std::vector<Route> ROUTES;

    class Server {
        public:
            void get(regex, Callback);
            void post(regex, Callback);
            void all(regex, Callback);
            void get(regex, string);
            void post(regex, string);
            void all(regex, string);
            void start(int, string, std::atomic<bool> &stop_flag);
            void start(int, string);
            void start(int);
            void start();
        private:
            void main_loop(void*, std::atomic<bool> &stop_flag);
            void parse_headers(char*, Request*, Response*);
            bool match_route(Request*, Response*);
            string trim(string);
            void split(string, string, int, vector<string>*);
    };

    void Server::split(string str, string separator, int max, vector<string>* results){
        int i = 0;
        size_t found = str.find_first_of(separator);

        while(found != string::npos){
            if(found > 0){
                results->push_back(str.substr(0, found));
            }
            str = str.substr(found+1);
            found = str.find_first_of(separator);

            if(max > -1 && ++i == max) break;
        }

        if(str.length() > 0){
            results->push_back(str);
        }
    }

    string Server::trim(string s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not_fn(static_cast<int(*)(int)>(std::isspace))));
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not_fn(static_cast<int(*)(int)>(std::isspace))).base(), s.end());

        return s;
    }

    void Server::parse_headers(char* headers, Request* req, Response* res) {
        // Parse request headers
        int i = 0;
        char * pch;
        for (pch = strtok(headers, "\n"); pch; pch = strtok(NULL, "\n" ))
        {
            if(i++ == 0)  {
                vector<string> R;
                string line(pch);
                this->split(line, " ", 3, &R);

    //            cout << R.size() << endl;

                if(R.size() != 3) {
    //                throw error
                }

                req->method = R[0];
                req->path = R[1];

                size_t pos = req->path.find('?');
                
                // We have GET params here
                if(pos != string::npos)  {
                    vector<string> Q1;
                    this->split(req->path.substr(pos + 1), "&", -1, &Q1);

                    for(vector<string>::size_type q = 0; q < Q1.size(); q++) {
                        vector<string> Q2;
                        this->split(Q1[q], "=", -1, &Q2);

                        if(Q2.size() == 2) {
                            req->query[Q2[0]] = Q2[1];
                        } else if(Q2.size() == 1) {
                            req->query[Q2[0]] = "";
                        }
                    }

                    req->path = req->path.substr(0, pos);
                }
            } else {
                vector<string> R;
                string line(pch);
                this->split(line, ": ", 2, &R);

                if(R.size() == 2) {
                    req->headers[R[0]] = R[1];
                    
                    // Yeah, cookies!
                    if(R[0] == "Cookie") {
                        vector<string> C1;
                        this->split(R[1], "; ", -1, &C1);
                        
                        for(vector<string>::size_type c = 0; c < C1.size(); c++) {
                            vector<string> C2;
                            this->split(C1[c], "=", 2, &C2);

                            if (C2.size() == 2) {
                                req->cookies[C2[0]] = C2[1];
                            }
                        }
                    }
                }
            }
        }
    }

    void Server::get(regex path, Callback callback) {
        Route r = {
             path,
             "GET",
             callback
        };

        ROUTES.push_back(r);
    }

    void Server::post(regex path, Callback callback) {
        Route r = {
             path,
             "POST",
             callback
        };

        ROUTES.push_back(r);
    }

    void Server::all(regex path, Callback callback) {
        Route r = {
             path,
             "ALL",
             callback
        };

        ROUTES.push_back(r);
    }

    void Server::get(regex path, string loc) {
        Route r = {
             path,
             "GET",
             &list_dir,
             loc
        };

        ROUTES.push_back(r);
    }

    void Server::post(regex path, string loc) {
        Route r = {
             path,
             "POST",
             &list_dir,
             loc
        };

        ROUTES.push_back(r);
    }

    void Server::all(regex path, string loc) {
        Route r = {
             path,
             "ALL",
             &list_dir,
             loc
        };

        ROUTES.push_back(r);
    }

    bool Server::match_route(Request* req, Response* res) {
        for (vector<Route>::size_type i = 0; i < ROUTES.size(); i++) {
            if(regex_match(req->path, ROUTES[i].path) && (ROUTES[i].method == req->method || ROUTES[i].method == "ALL")) {
                req->params = ROUTES[i].params;

                ROUTES[i].callback(req, res);

                return true;
            }
        }

        return false;
    }

    void Server::main_loop(void* arg, std::atomic<bool> &stop_flag) {
        int* port = reinterpret_cast<int*>(arg);

        int sc = socket(AF_INET, SOCK_STREAM, 0);
        int one = 1;
        setsockopt(sc, SOL_SOCKET, SO_REUSEADDR,  &one, sizeof(int));
        setsockopt(sc, SOL_SOCKET, SO_REUSEPORT,  &one, sizeof(int));

        if (sc < 0) {
            throw WPP::Exception("ERROR opening socket");
        }

        struct sockaddr_in serv_addr, cli_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(*port);

        if (::bind(sc, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
            throw WPP::Exception("ERROR on binding");
        }

        listen(sc, 5);

        socklen_t clilen;
        clilen = sizeof(cli_addr);
        int max_workers = std::thread::hardware_concurrency();
        max_workers = 1;
        std::atomic<int> current_workers(0);

        while(!stop_flag.load(std::memory_order_acquire)) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(sc, &read_fds);
            timeval timeout{.tv_sec=0, .tv_usec=100 };
            auto ready = select(sc+1, &read_fds, nullptr, nullptr, &timeout);
            if (ready==0) continue;
            if (ready==-1) abort();

            int newsc = accept(sc, (struct sockaddr *) &cli_addr, &clilen);

#if !(__linux__)
            {
                int option_value = 1; /* Set NOSIGPIPE to ON */
                if (setsockopt (newsc, SOL_SOCKET, SO_NOSIGPIPE, &option_value, sizeof(option_value)) < 0) {
                    perror("setsockopt(,,SO_NOSIGPIPE)");
                }
            }
#endif

            if (newsc < 0) {
                throw WPP::Exception("ERROR on accept");
            }

            while (current_workers >= max_workers) {
                usleep(1000);
            }

            current_workers++;

            FWThread worker_thread("web++worker", [newsc, this, &current_workers](){
                // handle new connection
                Request req;
                Response res;

                char headers[BUFSIZE + 1];
                long ret = read(newsc, headers, BUFSIZE);
                if(ret > 0 && ret < BUFSIZE) {
                    headers[ret] = 0;
                } else {
                    headers[0] = 0;
                }

                this->parse_headers(headers, &req, &res);

                if(!this->match_route(&req, &res)) {
                    res.code = 404;
                    res.phrase = "Not Found";
                    res.type = "text/plain";
                    res.send("Not found");
                }

                char header_buffer[BUFSIZE];
                string body = res.body.str();
                size_t body_len = body.length();

                // build http response
                sprintf(header_buffer, "HTTP/1.0 %d %s\r\n", res.code, res.phrase.c_str());

                // append headers
                sprintf(&header_buffer[strlen(header_buffer)], "Server: %s %s\r\n", SERVER_NAME, SERVER_VERSION);
                sprintf(&header_buffer[strlen(header_buffer)], "Date: %s\r\n", res.date.c_str());
                sprintf(&header_buffer[strlen(header_buffer)], "Content-Type: %s\r\n", res.type.c_str());
                sprintf(&header_buffer[strlen(header_buffer)], "Content-Length: %zd\r\n", body_len);

                for (const auto &header_pair : res.extra_headers) {
                    const auto &header_name = header_pair.first;
                    const auto &header_val = header_pair.second;
                    sprintf(&header_buffer[strlen(header_buffer)], "%s: %s\r\n",
                            header_name.c_str(), header_val.c_str());
                }

                // append extra crlf to indicate start of body
                strcat(header_buffer, "\r\n");

                ssize_t t;
                t = write(newsc, header_buffer, strlen(header_buffer));
                t = write(newsc, body.c_str(), body_len);
                close (newsc);
                current_workers--;
            });
            worker_thread.detach();
        }

        close(sc);
    }

    void Server::start(int port, string host, std::atomic<bool> &stop_flag) {
        this->main_loop(&port, stop_flag);
    }

    void Server::start(int port, string host) {
//         pthread_t worker;

//         for(int i = 0; i < 1; ++i) {
//              int rc = pthread_create (&worker, NULL, &mainLoop, NULL);
//              assert (rc == 0);
//         }

        std::atomic<bool> f(false);
        this->start(port, host, f);
    }

    void Server::start(int port) {
         this->start(port, "0.0.0.0");
    }

    void Server::start() {
         this->start(80);
    }
}
