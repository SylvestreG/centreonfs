//
// Created by syl on 10/10/19.
//

#include "centreonfs.hh"
#include "Fuse-impl.h"
#include <centreon-api.h>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static const std::string root_path{"/"};
static const std::string host_path{"/hosts"};
static const std::string service_path{"/services"};

std::set<std::string> paths{host_path, service_path};

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

crestapi::hosts::const_iterator get_host(const char *path) {
  crestapi::api &api{crestapi::api::instance()};

  std::string p{path};
  if (p.find(host_path) != std::string::npos) {
    std::string h{p.substr(host_path.size() + 1)};
    return api.get_hosts().find(h);
  }

  return api.get_hosts().end();
}

crestapi::services::const_iterator get_services(const char *path) {
  crestapi::api &api{crestapi::api::instance()};

  std::string p{path};
  if (p.find(service_path) != std::string::npos) {
    std::string s{p.substr(service_path.size() + 1)};
    std::vector<std::string> tupple{split(s, ',')};
    std::cout << "first => " << tupple[0] << std::endl;
    std::cout << "second => " << tupple[1] << std::endl;
    return api.get_services().find({tupple[0], tupple[1]});
  }

  return api.get_services().end();
}

int centreonfs::getattr(const char *path, struct stat *st,
                        struct fuse_file_info *ffi) {
  crestapi::api &api{crestapi::api::instance()};
  int res;

  std::cout << "[\031[1;33mcentreonfs\033[0m]: "
            << "lookup" << path << std::endl;

  memset(st, 0, sizeof(*st));
  if (path == root_path) {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
    std::cout << "[\031[1;33mcentreonfs\033[0m]: "
              << "find /" << std::endl;
    return 0;
  } else {
    for (auto &p : paths) {
      if (p == path) {
        std::cout << "[\031[1;33mcentreonfs\033[0m]: "
                  << "find p = path" << std::endl;
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
        return 0;
      }
    }
  }

  auto const &h{get_host(path)};
  if (h != api.get_hosts().end()) {
    st->st_mode = S_IFREG | 0444;
    st->st_nlink = 1;
    st->st_size = h->second.size();
    std::cout << "[\031[1;33mcentreonfs\033[0m]: "
              << "a" << std::endl;
    return 0;
  }

  std::cout << "here" << std::endl;
  auto const &s{get_services(path)};
  if (s != api.get_services().end()) {
    st->st_mode = S_IFREG | 0444;
    st->st_nlink = 1;
    st->st_size = s->second.size();
    std::cout << "[\031[1;33mcentreonfs\033[0m]: "
              << "b" << std::endl;
    return 0;
  }

  std::cout << "[\031[1;33mcentreonfs\033[0m]: "
            << "cannot find :" << path << std::endl;

  return -ENOENT;
}

int centreonfs::readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags) {
  if (paths.find(path) == paths.end() && path != root_path) {
    return -ENOENT;
  }

  crestapi::api &api{crestapi::api::instance()};

  filler(buf, ".", NULL, 0, FUSE_FILL_DIR_PLUS);
  filler(buf, "..", NULL, 0, FUSE_FILL_DIR_PLUS);
  if (root_path == path) {
    for (std::string const &dir : paths)
      if (dir.length() > 1)
        filler(buf, dir.c_str() + 1, NULL, 0, FUSE_FILL_DIR_PLUS);
  }
  if (host_path == path) {
    for (auto &h : api.get_hosts())
      filler(buf, h.first.c_str(), NULL, 0, FUSE_FILL_DIR_PLUS);
  }

  if (service_path == path) {
    for (auto &s : api.get_services())
      filler(buf, (s.first.first + ',' + s.first.second).c_str(), NULL, 0,
             FUSE_FILL_DIR_PLUS);
  }

  return 0;
}

int centreonfs::open(char const *path, struct fuse_file_info *fi) {
  crestapi::api &api{crestapi::api::instance()};
  auto const &h{get_host(path)};

  if (h != api.get_hosts().end()) {
    if ((fi->flags & 3) != O_RDONLY)
      return -EACCES;

    return 0;
  }

  auto const &s{get_services(path)};
  if (s != api.get_services().end()) {
    if ((fi->flags & 3) != O_RDONLY)
      return -EACCES;

    return 0;
  }

  return -ENOENT;
}

int centreonfs::read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
  crestapi::api &api{crestapi::api::instance()};
  auto const &h{get_host(path)};

  if (h != api.get_hosts().end()) {
    if (offset + size > h->second.length())
      size = h->second.length() - offset;

    memcpy(buf, &h->second.c_str()[offset], size);
    return size;
  }

  std::cout << "read service" << std::endl;
  auto const &s{get_services(path)};
  if (s != api.get_services().end()) {
    if (offset + size > s->second.length())
      size = s->second.length() - offset;

    memcpy(buf, &s->second.c_str()[offset], size);
    return size;
  }
  return 0;
}