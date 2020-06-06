"オートインデントする
:set autoindent
" ックスペースでインデントや改行を削除できるようにする。　
set backspace=2
set mouse-=a
set showcmd
set cmdheight=2
set wildmenu
set tabstop=4
set shiftwidth=4
set expandtab
augroup fileTypeIndent
    autocmd!
    autocmd FileType ruby setl ts=2 shiftwidth=2
augroup END
