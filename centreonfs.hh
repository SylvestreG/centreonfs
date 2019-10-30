//
// Created by syl on 10/10/19.
//

#ifndef CENTREONFS_CMAKE_BUILD_DEBUG_CENTREONFS_HH_
#define CENTREONFS_CMAKE_BUILD_DEBUG_CENTREONFS_HH_

#include "Fuse-impl.h"
#include "Fuse.h"

class centreonfs : public Fusepp::Fuse<centreonfs> {
public:
  centreonfs() {}
  ~centreonfs() {}

  static int getattr(char const *, struct stat *, struct fuse_file_info *);
  static int readdir(char const *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi,
                     enum fuse_readdir_flags);

  static int open(char const *path, struct fuse_file_info *fi);
  static int read(char const *path, char *buf, size_t size, off_t offset,
                  struct fuse_file_info *fi);
};

#endif // CENTREONFS_CMAKE_BUILD_DEBUG_CENTREONFS_HH_
