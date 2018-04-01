#include "repo.h"
#include <libgit2/include/git2.h>

namespace Git {
class LibGitWrapper{
public:
    static void init(){
        static bool initialized=false;
        if (!initialized){
              git_libgit2_init();
              initialized=true;
        }
    }
};

Repo::Repo(FileInfo &repodir)
{
repodir.;
}
}

