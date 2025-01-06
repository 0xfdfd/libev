var group__EV__FILESYSTEM =
[
    [ "ev_file_s", "group__EV__FILESYSTEM.html#structev__file__s", [
      [ "base", "group__EV__FILESYSTEM.html#a0eb53837eb049ab9bfb987bdc03c7fe4", null ],
      [ "close_cb", "group__EV__FILESYSTEM.html#addd4d03933d2b3ad42afc5b9fc63ccab", null ],
      [ "file", "group__EV__FILESYSTEM.html#a112af2907a5080b76afd091cdcfe880b", null ],
      [ "work_queue", "group__EV__FILESYSTEM.html#a704638632b2f84b7edbd65475714af37", null ]
    ] ],
    [ "ev_file_map", "group__EV__FILESYSTEM.html#structev__file__map", [
      [ "addr", "group__EV__FILESYSTEM.html#ae5bd6c22dbf0f6b5b0ae0233f8eb3704", null ],
      [ "backend", "group__EV__FILESYSTEM.html#a5a54be94abc55672ff4b1e33e60700d5", null ],
      [ "size", "group__EV__FILESYSTEM.html#af931a8871310b4dad23f0f0b0f623560", null ]
    ] ],
    [ "ev_fs_stat_s", "group__EV__FILESYSTEM.html#structev__fs__stat__s", [
      [ "st_atim", "group__EV__FILESYSTEM.html#a091257f26851140458b4d4c2dfb2beeb", null ],
      [ "st_birthtim", "group__EV__FILESYSTEM.html#aab97a1e647196b5ac8a46764b01a7224", null ],
      [ "st_blksize", "group__EV__FILESYSTEM.html#adb9252bb3343471e4aaaf7bf7df5363a", null ],
      [ "st_blocks", "group__EV__FILESYSTEM.html#a72c24b54949be7eaac849d71fc056a3c", null ],
      [ "st_ctim", "group__EV__FILESYSTEM.html#a24d4bc66f6d41ade09776ae461725baf", null ],
      [ "st_dev", "group__EV__FILESYSTEM.html#a0146849d3da5c91d9776888b14b82dcc", null ],
      [ "st_flags", "group__EV__FILESYSTEM.html#a513d310cddd83b8f1678cf2a8ca16714", null ],
      [ "st_gen", "group__EV__FILESYSTEM.html#a67c4e0f707b80853bb37d589136c00f0", null ],
      [ "st_gid", "group__EV__FILESYSTEM.html#a09c579712d49de2a34f3ec9d7fdaa850", null ],
      [ "st_ino", "group__EV__FILESYSTEM.html#abb54f0e7d91659ba9f4ab77a5392c08a", null ],
      [ "st_mode", "group__EV__FILESYSTEM.html#a5b4e70e604ddbe3fff52f27f9fb3df88", null ],
      [ "st_mtim", "group__EV__FILESYSTEM.html#aca174ee97c6451a15c5582a4d5fdf4c7", null ],
      [ "st_nlink", "group__EV__FILESYSTEM.html#abef7057bf9bf605a14cd64d34cf3424d", null ],
      [ "st_rdev", "group__EV__FILESYSTEM.html#ac430407fd3b0e421da1ee8f66c95a786", null ],
      [ "st_size", "group__EV__FILESYSTEM.html#ae9a97f03571901a066d512b87bc36dba", null ],
      [ "st_uid", "group__EV__FILESYSTEM.html#aa5d3e5f4a119ff845649c779062ce1f3", null ]
    ] ],
    [ "ev_dirent_s", "group__EV__FILESYSTEM.html#structev__dirent__s", [
      [ "name", "group__EV__FILESYSTEM.html#a5ac083a645d964373f022d03df4849c8", null ],
      [ "type", "group__EV__FILESYSTEM.html#a4de355e2721ef8c23c93565f769ec766", null ]
    ] ],
    [ "ev_fs_req_s", "group__EV__FILESYSTEM.html#structev__fs__req__s", [
      [ "cb", "group__EV__FILESYSTEM.html#acde35845b6beb81e961d74e0425a1782", null ],
      [ "file", "group__EV__FILESYSTEM.html#aa2f6cc8a3a5f576a58fa400ea657c818", null ],
      [ "node", "group__EV__FILESYSTEM.html#aad11ebad3f26bf69a1bb5968fc63a247", null ],
      [ "req_type", "group__EV__FILESYSTEM.html#a458d84a24881aec99a6c65ab36ba02ae", null ],
      [ "result", "group__EV__FILESYSTEM.html#aee444f666b09201be448a9b92371fc4f", null ],
      [ "work_token", "group__EV__FILESYSTEM.html#a5db00589ff7f1564f3d299745e8e9e04", null ]
    ] ],
    [ "ev_fs_req_s.req", "group__EV__FILESYSTEM.html#unionev__fs__req__s_8req", null ],
    [ "ev_fs_req_s.req.as_open", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__open", [
      [ "flags", "group__EV__FILESYSTEM.html#a4e5868d676cb634aa75b125a0f741abf", null ],
      [ "mode", "group__EV__FILESYSTEM.html#a15d61712450a686a7f365adf4fef581f", null ],
      [ "path", "group__EV__FILESYSTEM.html#ad6fe1d0be6347b8ef2427fa629c04485", null ]
    ] ],
    [ "ev_fs_req_s.req.as_seek", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__seek", [
      [ "offset", "group__EV__FILESYSTEM.html#a7a86c157ee9713c34fbd7a1ee40f0c5a", null ],
      [ "whence", "group__EV__FILESYSTEM.html#a02c86cc535151887678873a6d0a3c347", null ]
    ] ],
    [ "ev_fs_req_s.req.as_read", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__read", [
      [ "offset", "group__EV__FILESYSTEM.html#a7a86c157ee9713c34fbd7a1ee40f0c5a", null ],
      [ "read_req", "group__EV__FILESYSTEM.html#ae8eb0e5f5e1a3d35b49f7beda5ab5269", null ]
    ] ],
    [ "ev_fs_req_s.req.as_write", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__write", [
      [ "offset", "group__EV__FILESYSTEM.html#a7a86c157ee9713c34fbd7a1ee40f0c5a", null ],
      [ "write_req", "group__EV__FILESYSTEM.html#a100a30494c229182e15617ee3f4db7c4", null ]
    ] ],
    [ "ev_fs_req_s.req.as_readdir", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__readdir", [
      [ "path", "group__EV__FILESYSTEM.html#ad6fe1d0be6347b8ef2427fa629c04485", null ]
    ] ],
    [ "ev_fs_req_s.req.as_readfile", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__readfile", [
      [ "path", "group__EV__FILESYSTEM.html#ad6fe1d0be6347b8ef2427fa629c04485", null ]
    ] ],
    [ "ev_fs_req_s.req.as_mkdir", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__mkdir", [
      [ "mode", "group__EV__FILESYSTEM.html#a15d61712450a686a7f365adf4fef581f", null ],
      [ "path", "group__EV__FILESYSTEM.html#ad6fe1d0be6347b8ef2427fa629c04485", null ]
    ] ],
    [ "ev_fs_req_s.req.as_remove", "group__EV__FILESYSTEM.html#structev__fs__req__s_8req_8as__remove", [
      [ "path", "group__EV__FILESYSTEM.html#ad6fe1d0be6347b8ef2427fa629c04485", null ],
      [ "recursion", "group__EV__FILESYSTEM.html#af8532b6380228945d474f37fb6cdac27", null ]
    ] ],
    [ "ev_fs_req_s.rsp", "group__EV__FILESYSTEM.html#unionev__fs__req__s_8rsp", [
      [ "dirents", "group__EV__FILESYSTEM.html#a7b84e17e8248bdb3177a4a8f4ddde025", null ],
      [ "filecontent", "group__EV__FILESYSTEM.html#a86c0460e2f881850a75eac2f5e0b7d07", null ],
      [ "stat", "group__EV__FILESYSTEM.html#a77ddcb5f19832f4145345889013ab3a4", null ]
    ] ],
    [ "ev_dirent_t", "group__EV__FILESYSTEM.html#ga6ae489a03f35fef172078c3ebf92c0bc", null ],
    [ "ev_dirent_type_t", "group__EV__FILESYSTEM.html#gaa77a55c2e681f26bdf07161a051a1ae4", null ],
    [ "ev_file_cb", "group__EV__FILESYSTEM.html#ga8c312226577eb01565d88736c67c460a", null ],
    [ "ev_file_close_cb", "group__EV__FILESYSTEM.html#gae06a9a4a1e14756c22d4c004a5ae5212", null ],
    [ "ev_file_t", "group__EV__FILESYSTEM.html#ga00c0feeb8883df5988360e0d3cbd43ef", null ],
    [ "ev_fs_req_t", "group__EV__FILESYSTEM.html#gae7a0c42ce306b94c5cfad60ed2a2a071", null ],
    [ "ev_fs_req_type_t", "group__EV__FILESYSTEM.html#ga69252665b0f487234753384bebc0c6a9", null ],
    [ "ev_fs_stat_t", "group__EV__FILESYSTEM.html#ga9a4729e0856a450f1e8ade5d8aaaf392", null ],
    [ "ev_dirent_type_e", "group__EV__FILESYSTEM.html#gac45d90532510222c6012f2d2829b614a", null ],
    [ "ev_fs_req_type_e", "group__EV__FILESYSTEM.html#ga5e5305fef033f66213d8133028d7c069", null ],
    [ "ev_file_close", "group__EV__FILESYSTEM.html#ga932e487de118760fa5f3461580312699", null ],
    [ "ev_file_mmap", "group__EV__FILESYSTEM.html#ga808a00a75c13e6e0ea1cfae21308482d", null ],
    [ "ev_file_munmap", "group__EV__FILESYSTEM.html#ga88e9ea67b07aa37c32a259da25c71514", null ],
    [ "ev_file_open", "group__EV__FILESYSTEM.html#ga80683d54949c65bcd1daecd32204481e", null ],
    [ "ev_file_pread", "group__EV__FILESYSTEM.html#gaf6f587f580aee0dd8920481d520b31fc", null ],
    [ "ev_file_preadv", "group__EV__FILESYSTEM.html#ga6b9f0854429937774ca25767e79b5bc1", null ],
    [ "ev_file_pwrite", "group__EV__FILESYSTEM.html#ga6c47425ae3722a446ed9c9edf89eaeb6", null ],
    [ "ev_file_pwritev", "group__EV__FILESYSTEM.html#gaf54fe4dd6815f4c8605bfa8b2599c61b", null ],
    [ "ev_file_read", "group__EV__FILESYSTEM.html#gaf7c33caa215f7aa6873833af98aefbb3", null ],
    [ "ev_file_readv", "group__EV__FILESYSTEM.html#ga420a2f23ad7d0633989f8a050e40fbed", null ],
    [ "ev_file_seek", "group__EV__FILESYSTEM.html#ga3bca482d0cafcb37c532f6122ad365a2", null ],
    [ "ev_file_stat", "group__EV__FILESYSTEM.html#ga474ad7a662419d48a993427ae1aef44e", null ],
    [ "ev_file_write", "group__EV__FILESYSTEM.html#ga7835cd161a151c24e336edf6da675bfd", null ],
    [ "ev_file_writev", "group__EV__FILESYSTEM.html#ga9d46e64cb73ff3186792dc7f6171a2e0", null ],
    [ "ev_fs_get_file", "group__EV__FILESYSTEM.html#ga163417eb667b4365b6de3c5cfc7b10ac", null ],
    [ "ev_fs_get_filecontent", "group__EV__FILESYSTEM.html#ga3addcd3e8ccba694ebd0bba40972d795", null ],
    [ "ev_fs_get_first_dirent", "group__EV__FILESYSTEM.html#gab1d80d5d4793a9adb3cf438b8f80c690", null ],
    [ "ev_fs_get_next_dirent", "group__EV__FILESYSTEM.html#ga33352c15c694c23ec8604b3a185f3396", null ],
    [ "ev_fs_get_statbuf", "group__EV__FILESYSTEM.html#ga9349c0d60c02ebf57187f9a050e15933", null ],
    [ "ev_fs_mkdir", "group__EV__FILESYSTEM.html#ga61c16964d0d3a8a4b0a41694015fa7c4", null ],
    [ "ev_fs_readdir", "group__EV__FILESYSTEM.html#ga07d9cb368a2e281a2c70bc838f479131", null ],
    [ "ev_fs_readfile", "group__EV__FILESYSTEM.html#gad30f5c2bba57318241ce4927873f6c9f", null ],
    [ "ev_fs_remove", "group__EV__FILESYSTEM.html#ga6cfe1862c697830f7b34a72be82df1e3", null ],
    [ "ev_fs_req_cleanup", "group__EV__FILESYSTEM.html#gab83006c25c3b579450a85b80ad642148", null ]
];