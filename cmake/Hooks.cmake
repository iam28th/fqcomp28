add_custom_target(hooks)

# "map" hook name to a script path
set(hooks_kv_pairs
    "pre-commit" ${CMAKE_SOURCE_DIR}/scripts/pre-commit.sh
    "pre-push" ${CMAKE_SOURCE_DIR}/scripts/pre-push.sh
)
list(LENGTH hooks_kv_pairs hooks_length)
math(EXPR end_idx_ "${hooks_length} - 2")


# loop over indexes for hook name and its script
foreach(key_idx RANGE 0 ${end_idx_} 2)
    math(EXPR val_idx "${key_idx} + 1")

    list(GET hooks_kv_pairs ${key_idx} name)
    list(GET hooks_kv_pairs ${val_idx} src)

    # construct destination path from name
    set(dst ${CMAKE_SOURCE_DIR}/.git/hooks/${name})

    # create a symlink after the target is "build"
    add_custom_command(TARGET hooks
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${src} ${dst}
    )
endforeach()
