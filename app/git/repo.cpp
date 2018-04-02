#include "repo.h"
#include <QDebug>
#include <vector>
#include <git2.h>


void GitRepo::lib_init(){
    static bool initialized=false;
    if (!initialized){
        git_libgit2_init();
        initialized=true;
    }
}

GitRepo::GitRepo():repo_path(nullptr),repo(nullptr),index(nullptr){
    lib_init();
}

bool GitRepo::open_or_initialize(const QFileInfo& repo_path_){

    repo_path = new QFileInfo(repo_path_);
    auto repoUTF8 = repo_path->filePath().toUtf8();
    const char* path = repoUTF8.constData();

    const git_repository_open_flag_t OPEN_FLAGS =  GIT_REPOSITORY_OPEN_NO_SEARCH;

    auto ceiling_dirs=nullptr;//because of the above flag, don't need ceiling_dirs
    auto open_err= git_repository_open_ext(&repo,path,OPEN_FLAGS,ceiling_dirs);
    if(open_err==GIT_ENOTFOUND){//GIT_ENOTFOUND if no repository could be found
        auto init_err=git_repository_init(&repo,path,0);
        if(init_err!=0){
            qDebug()<<giterr_last();
            return false;
        }

    }else if(open_err==-1){//-1 if there was a repository but open failed for some reason (such as repo corruption or system errors).
        qDebug()<<giterr_last();
        return false;
    }//0 on success,

    auto repo_index_ret = git_repository_index(&index, repo);
    if (repo_index_ret!=0){
        qDebug()<<giterr_last();
        return false;
    }
    return true;

}
struct status_cb_payload{
    git_repository *repo;
    git_index *index;
};

int addAll_cb(const char *path, unsigned int status_flags, void *payload){
    (void)status_flags;
    int ignored;
    const status_cb_payload *pl=static_cast<status_cb_payload*>(payload);
    git_status_should_ignore(&ignored,pl->repo,path);
    if(!ignored){
        git_index_add_bypath(pl->index,path);
    }
    return 0;
}

bool GitRepo::addAll(){

    //TODO: check if we need to add glob
    char *paths[]={"*"};
    git_strarray pathspec = {paths, 1};

    auto update_cb=nullptr;
    //loop through all status files, add unadded to the index if not in .ignore file
    status_cb_payload payload ={repo,index};
    auto add_err = git_status_foreach(repo,addAll_cb,&payload);
    if(add_err!=0){
        qDebug()<<giterr_last();
        return false;
    }

    auto update_err = git_index_update_all(index,&pathspec,update_cb,nullptr);
    if(update_err!=0){
        qDebug()<<giterr_last();
        return false;
    }
    auto write_err=git_index_write(index);
    if(write_err!=0){
        qDebug()<<giterr_last();
        return false;
    }

    return true;
}

bool GitRepo::no_init(){
    return repo_path==nullptr;
}


bool GitRepo::commit(const QString& message, const QString& name,const QString& email){
    /*
         * Build the tree from the index
         */

    //get oid
    git_oid tree_oid;
    git_index_write_tree(&tree_oid, index);
    /*
         * Commit the staged file
         */
    git_signature *signature=nullptr;
    const auto name_str=name.toUtf8();
    const auto email_str=email.toUtf8();
    const auto sig_err= git_signature_now(&signature, name_str.constData(), email_str.constData());
    if(sig_err!=0){
        qDebug()<<giterr_last();
    }

    git_tree *tree=nullptr;
    git_tree_lookup(&tree, repo, &tree_oid);

    //make sure message conforms to git formatting (e.g. trailing newline)
    git_buf buffer;
    memset(&buffer, 0, sizeof(git_buf));
    const auto message_str = message.toUtf8();
    git_message_prettify(&buffer, message_str.constData(), 0, '#');

    //look up HEAD as parent, if not unborn
    std::vector<git_commit*> parents;
    if(git_repository_head_unborn(repo)){
        //is initial commit, no parents
    }else{
        git_commit *parent=NULL;
        git_oid parent_id;
        git_reference_name_to_id(&parent_id,repo,"HEAD");
        git_commit_lookup(&parent, repo, &parent_id);
        parents.push_back(parent);
    }

    git_oid new_commit_oid;;
    const auto parent_count=parents.size();
    const git_commit** parents_ptr=parents.size()==0?NULL:const_cast<const git_commit**>(parents.data());
    const auto create_err = git_commit_create(
                &new_commit_oid,
                repo,
                "HEAD",
                signature,
                signature,
                "UTF-8",
                buffer.ptr,
                tree,
                parent_count,
                parents_ptr);
    bool ret=false;
    if(create_err!=0){
        qDebug()<<giterr_last();
        ret=false;
    }else{
        ret=true;
    }

    git_buf_free(&buffer);
    git_signature_free(signature);
    git_tree_free(tree);
    return ret;
}

GitRepo::~GitRepo(){
    if(repo_path!=nullptr){
        delete repo_path;
        repo_path=nullptr;
    }
    if(repo!=nullptr){
        git_repository_free(repo);
        repo=nullptr;
    }
    if(index!=nullptr){
        git_index_free(index);
        index=nullptr;
    }

}



