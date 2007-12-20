#include <assert.h>
#include <db_cxx.h>

DbEnv::DbEnv (u_int32_t flags)
    : do_no_exceptions((flags&DB_CXX_NO_EXCEPTIONS)!=0)
{
    int ret = db_env_create(&the_env, flags & ~DB_CXX_NO_EXCEPTIONS);
    assert(ret==0); // should do an error.
    the_env->api1_internal = this;
}

DbEnv::DbEnv(DB_ENV *env, u_int32_t flags)
    : do_no_exceptions((flags&DB_CXX_NO_EXCEPTIONS)!=0)
{
    the_env = env;
    if (env == 0) {
	DB_ENV *new_env;
	int ret = db_env_create(&new_env, flags & ~DB_CXX_NO_EXCEPTIONS);
	assert(ret==0); // should do an error.
	the_env = new_env;
    }
    the_env->api1_internal = this;
}

int DbEnv::close(u_int32_t flags) {
    int ret = the_env->close(the_env, flags);
    the_env = 0; /* get rid of the env ref, so we don't touch it (even if we failed.) */
    return maybe_throw_error(ret);
}

int DbEnv::open(const char *home, u_int32_t flags, int mode) {
    int ret = the_env->open(the_env, home, flags, mode);
    return maybe_throw_error(ret);
}

int DbEnv::set_cachesize(u_int32_t gbytes, u_int32_t bytes, int ncache) {
    int ret = the_env->set_cachesize(the_env, gbytes, bytes, ncache);
    return maybe_throw_error(ret);
}

#if DB_VERSION_MAJOR<4 || (DB_VERSION_MAJOR==4 && DB_VERSION_MINOR<=4)
int DbEnv::set_lk_max(u_int32_t flags) {
    int ret = the_env->set_lk_max(the_env, flags);
    return maybe_throw_error(ret);
}
#endif

int DbEnv::txn_begin(DbTxn *parenttxn, DbTxn **txnp, u_int32_t flags) {
    DB_TXN *txn;
    int ret = the_env->txn_begin(the_env, parenttxn->get_DB_TXN(), &txn, flags);
    if (ret==0) {
	*txnp = new DbTxn(txn);
    }
    return maybe_throw_error(ret);
}

int DbEnv::set_data_dir(const char *dir) {
    int ret = the_env->set_data_dir(the_env, dir);
    return maybe_throw_error(ret);
}

void DbEnv::set_errpfx(const char *errpfx) {
    the_env->set_errpfx(the_env, errpfx);
}

int DbEnv::maybe_throw_error(int err) {
    if (err==0) return 0;
    if (do_no_exceptions) return err;
    DbException e(err);
    e.set_env(the_env);
    throw e;
}
