" auto-instll vim-plug
if empty(glob('~/.config/nvim/autoload/plug.vim'))
  silent !curl -fLo ~/.config/nvim/autoload/plug.vim --create-dirs
    \ https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  "autocmd VimEnter * PlugInstall
  "autocmd VimEnter * PlugInstall | source $MYVIMRC
endif

call plug#begin('~/.config/nvim/autoload/plugged/')

    Plug 'junegunn/fzf', { 'do': { -> fzf#install() } }
    Plug 'junegunn/fzf.vim'
    Plug 'tpope/vim-surround'
    Plug 'Raimondi/delimitMate'
    Plug 'vim-airline/vim-airline'
    Plug 'vim-syntastic/syntastic'
    Plug 'maxboisvert/vim-simple-complete' 
    Plug 'vim-airline/vim-airline-themes'
    Plug 'rafi/awesome-vim-colorschemes'
    Plug 'preservim/nerdcommenter'
    Plug 'ervandew/supertab'
    " Better Syntax Support
    Plug 'sheerun/vim-polyglot'
    " File Explorer
    Plug 'scrooloose/NERDTree'
    " Auto pairs for '(' '[' '{'
    Plug 'jiangmiao/auto-pairs'
    "Colorsheme"
    Plug 'marko-cerovac/material.nvim'
    Plug 'glepnir/dashboard-nvim'
    Plug 'hoob3rt/lualine.nvim'
    " If you want to have icons in your statusline choose one of these
    "Plug 'kyazdani42/nvim-web-devicons'
    "Plug 'ryanoasis/vim-devicons'
    Plug 'dracula/vim', { 'as': 'dracula' }
    "Clap
    Plug 'liuchengxu/vim-clap'
    "Plug 'challenger-deep-theme/vim', {'as': 'challenger-deep'}
    call plug#end()
lua require('lualine').setup{options = {theme = 'dracula'}}

