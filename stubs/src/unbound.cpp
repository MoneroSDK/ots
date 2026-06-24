extern "C" {
    struct ub_ctx* ub_ctx_create(void) { return nullptr; }
    void ub_ctx_delete(struct ub_ctx*) { }
    int ub_ctx_zone_add(struct ub_ctx*, const char*) { return 0; }
    int ub_ctx_async(struct ub_ctx*, int) { return 0; }
}
