
#include "centreonfs.hh"
#include <centreon-api.h>
#include <fuse3/fuse_opt.h>
#include <iostream>
#include <sstream>

struct centreonfs_config {
  char const *hostname;
  uint16_t port;
  char const *user;
  char const *password;
};

enum {
  KEY_HELP,
  KEY_VERSION,
};

#define CENTREONFS_OPT(t, p, v)                                                \
  { t, offsetof(struct centreonfs_config, p), v }

static centreonfs fs;

static struct fuse_opt centreonfs_opts[] = {
    CENTREONFS_OPT("-p=%i", port, 0),
    CENTREONFS_OPT("-h=%s", hostname, 0),
    CENTREONFS_OPT("-u=%s", user, 0),
    CENTREONFS_OPT("-P=%s", password, 0),

    FUSE_OPT_KEY("-V", KEY_VERSION),
    FUSE_OPT_KEY("--version", KEY_VERSION),
    FUSE_OPT_KEY("-h", KEY_HELP),
    FUSE_OPT_KEY("--help", KEY_HELP),
    FUSE_OPT_END};

static int centreonfs_opt_proc(void *data, const char *arg, int key,
                               struct fuse_args *outargs) {
  switch (key) {
  case KEY_HELP:
    std::cerr << "usage: %s mountpoint [options]\n"
                 "\n"
                 "general options:\n"
                 "    -o opt,[opt...]  mount options\n"
                 "    -h   --help      print help\n"
                 "    -V   --version   print version\n"
                 "\n"
                 "Centreonfs options:\n"
                 "    -h hostname\n"
                 "    -p port\n"
                 "    -u username\n"
                 "    -P password\n\n";
    fuse_opt_add_arg(outargs, "-h");
    fs.run(outargs->argc, outargs->argv);
    exit(1);

  case KEY_VERSION:
    std::cerr << "centreonfs version 0.1" << std::endl;
    fuse_opt_add_arg(outargs, "--version");
    fs.run(outargs->argc, outargs->argv);
    exit(0);
  }
  return 1;
}

int main(int ac, char **av) {
  struct fuse_args args = FUSE_ARGS_INIT(ac, av);
  struct centreonfs_config conf;

  memset(&conf, 0, sizeof(conf));

  fuse_opt_parse(&args, &conf, centreonfs_opts, centreonfs_opt_proc);

  auto check_arg = [](auto &arg, auto value, std::string field_name) -> void {
    if (!arg) {
      std::cout << "[\033[1;33mcentreonfs\033[0m]: "
                << "No " << field_name << " provided using default(" << value
                << ")" << std::endl;
      arg = value;
    }
  };

  check_arg(conf.port, static_cast<uint16_t>(80), "port");
  check_arg(conf.hostname, "localhost", "127.0.0.1");
  check_arg(conf.user, "admin", "user");
  check_arg(conf.password, "centreon", "password");

  std::stringstream ss;
  ss << "http://" << conf.hostname << ":" << conf.port << "/centreon/api/beta";

  crestapi::api &api{crestapi::api::instance()};
  api.fetch(ss.str(), conf.user, conf.password);
  std::cout << "[\033[1;32mcentreonfs\033[0m]: "
            << "data fetch done" << std::endl;
  std::cout << "[\033[1;32mcentreonfs\033[0m]: "
            << "hosts: " << api.get_hosts().size() << std::endl;

  fs.run(ac, av);
  return 0;
}