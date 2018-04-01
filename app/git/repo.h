#ifndef REPO_H
#define REPO_H
#include <QtCore/qfileinfo.h>
#include <libgit2/include/git2.h>
namespace Git {
class LibgitRepoWrap{
    git_repository *repo;
    LibgitRepoWrap(const char* path){
        git_repository_open_flag_t OPEN_FLAGS=GIT_REPOSITORY_OPEN_NO_SEARCH;//do not walk up the filesystem to search for a repositor
        auto ceiling_dirs=nullptr;//because of the above flag, don't need ceiling_dirs
        auto open_err= git_repository_open_ext(repo,path,OPEN_FLAGS,ceiling_dirs);
        if(open_err==GIT_ENOTFOUND){//GIT_ENOTFOUND if no repository could be found
            auto init_err=git_repository_init_ext(repo,path,nullptr);
            if(init_err!=null){
                //TODO: check for errors
            }
        }else if(open_err==-1){//-1 if there was a repository but open failed for some reason (such as repo corruption or system errors).

        }//0 on success,

    }
    ~LibgitRepoWrap(){
        if(repo!=nullptr){
        git_repository_free(repo);
        repo=nullptr;
        }
    }
    void add(){
        /*
         * Add a new file to the index
         */
    git_repository_index(&index, repo);
    git_index_add_bypath(index, "test.txt");

    entry = git_index_get_byindex(index, 0);

    /*
     * Build the tree from the index
     */
    git_index_write_tree(&tree_oid, index);
    }
    void commit(){


        /*
             * Commit the staged file
             */
            cl_git_pass(git_signature_new(&signature, "nulltoken", "emeric.fermas@gmail.com", 1323847743, 60));
            cl_git_pass(git_tree_lookup(&tree, repo, &tree_oid));

            memset(&buffer, 0, sizeof(git_buf));
            cl_git_pass(git_message_prettify(&buffer, "Initial commit", 0, '#'));

            cl_git_pass(git_commit_create_v(
                &commit_oid,
                repo,
                "HEAD",
                signature,
                signature,
                NULL,
                buffer.ptr,
                tree,
                0));

            cl_assert(git_oid_cmp(&expected_commit_oid, &commit_oid) == 0);

            git_buf_free(&buffer);
            git_signature_free(signature);
            git_tree_free(tree);
        git_index_free(index);
    }

};
class Repo
{

public:
    Repo(QFileInfo &repodir);
};
}

#endif // REPO_H
