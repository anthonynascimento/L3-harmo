# Obfuscation de code

A partir de la procédure fournie par Valentin BOUQUET

## Documentations
- [Documentation officiel de l'API clang](https://clang.llvm.org/doxygen/namespaceclang.html#details)
- [Documentation officiel sur les différents outils de clang](https://clang.llvm.org/docs/index.html)
- [Documentation officiel des Ast matcher](http://clang.llvm.org/docs/LibASTMatchersReference.html)
- [Des exemples d'utilisations de LLVM et CLANG](https://github.com/eliben/llvm-clang-samples)
- [Clang ASTMatcher, ASTVisteur et réécriture de code](https://eli.thegreenplace.net/2014/07/29/ast-matchers-and-clang-refactoring-tools)
- [Liste des noeuds déclaration avec clang](https://clang.llvm.org/doxygen/classclang_1_1Decl.html)
- [Liste des noeuds statement avec clang](https://clang.llvm.org/doxygen/classclang_1_1Stmt.html)


## Commandes (utilitaire)

```
# Compiler vos fichiers clang
cd /vagrant
make

# Exécuter l'AST Visitor (rewriter)
build/clang_ast_visitor exemples/file.c -- -I/vagrant/libs/clang_llvm/lib/clang/3.9.1/include/

```
