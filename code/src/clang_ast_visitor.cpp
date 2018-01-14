//------------------------------------------------------------------------------
// Clang rewriter sample. Demonstrates:
//
// * How to use RecursiveASTVisitor to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <cstdio>
#include <memory>
#include <string>
#include <sstream>

#include <iostream>
#include <fstream>

#include <bitset> //Include supplémentaire

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace std;

// By implementing RecursiveASTVisitor, we can specify which AST nodes
// we're interested in by overriding relevant methods.
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
private:
  Rewriter &TheRewriter;

public:
    MyASTVisitor(Rewriter &R) :
        TheRewriter(R) {}

    // Déclaration de tous les prototypes visiteur
    bool VisitFunctionDecl(FunctionDecl *f);
    bool VisitVarDecl(VarDecl *v);
    bool VisitDeclRefExpr(DeclRefExpr *f);

};

//Fonction de traduction d'un texte en binaire
std::string stringToBinary(std::string text) {
  std::string resultat;
  for(std::size_t i = 0;i<text.size();i++) {
    std::string binaire;
    std::bitset<8> convert (text[i]);
    binaire = convert.to_string();
    for(std::size_t j = 0;j<binaire.size();j++) {
      if(binaire[j] == '0') binaire[j] = 'l';
    }
    resultat += binaire;
  }
  return resultat;
}

//Fonction de suppression des espaces inutiles
std::string suppressionEspace(std::string text) {
  int it=0;
  std::string salut;
  std::size_t i = 0;
  while(i<text.size() && text[i]==' ') {
    i++;
    it++;
  }
  for(std::size_t j = it;j<text.size();j++) {
      salut+=text[j];
  }
  return salut;
}

//Générations de l'aléatoire pour les commentaires
int generer_bornes(int min, int max) {
    static int rand_is_seeded = 0;
    if(!rand_is_seeded) {
        srand(time(NULL));
        rand_is_seeded = 1;
    }
    return rand()%(max-min+1) + min;
}

//Fonctions permettant d'enlever tout les sauts de ligne par l'utilisation d'un fichier intermédiaire
std::string creationFichier() {
  std::ifstream fichierA("/vagrant/output/file.c", std::ios::in); //ouverture de file.c en lecture
  std::ofstream fichierF("/vagrant/output/file-inter.c", std::ios::out);  // ouverture en écriture
  std::string contenu;
  std::string ligne;
  std::string ligneSE;
  if(fichierA) {
    while(getline(fichierA,ligne)) {
      ligneSE=suppressionEspace(ligne);
      if(ligneSE[0]=='#') fichierF << ligneSE << endl;
      else contenu+=ligneSE;
    }
    fichierF << contenu << endl;
  }
  else {
    std::cerr << "erreur avec le fichier" << std::endl;
  }
  fichierA.close();
  fichierF.close();
  return contenu;
}

std::string remplacementFichier(){
  std::ifstream fichierA("/vagrant/output/file-inter.c", std::ios::in); //ouverture en lecture
  std::ofstream fichierF("/vagrant/output/file.c", std::ios::out| std::ios::trunc);  // ouverture en écriture
  std::string contenu;
  std::string ligne;
  if(fichierA) {
    while(getline(fichierA,ligne)) {
      if(ligne[0]=='#') fichierF << ligne << endl;
      else{
        std::string instructions[5] = {"printf", "main", "argc", "argv", "scanf"};
        int i;
        std::string define;
        for(i=0;i<5;i++) {
          define = "#define "+stringToBinary(instructions[i]) + " "+ instructions[i];
          fichierF << define << endl;
        }
        contenu+=ligne;
      }
    }
    fichierF << contenu << endl;
  }
  else {
    std::cerr << "erreur avec le fichier" << std::endl;
  }
  fichierA.close();
  fichierF.close();
  if(remove("/vagrant/output/file-inter.c") != 0) {
    std::cerr << "erreur de suppression" << std::endl;
  }
  return contenu;
}

/*Insertion des commentaires*/
std::string lectureEtInsertionFichier() {
  std::ifstream fichier("/vagrant/src/test.txt", std::ios::in); //ouverture de test.txt en lecture
  std::string contenu;
  std::string ligne;
  int alea= generer_bornes(1,26);
  if(fichier) {
    int i=1;
    while(getline(fichier,ligne) && i<=alea) {
      if(i==alea) contenu+=ligne;
      i++;
    }
  }
  else {
    std::cerr << "erreur avec le fichier" << std::endl;
  }
  suppressionEspace(contenu);
  return contenu;
}

/**
 * Implémentation d'un visiteur, parcourant les noeuds de type FunctionDecl
**/
bool MyASTVisitor::VisitFunctionDecl(FunctionDecl *f) {
    DeclarationNameInfo DeclNameInfo = f->getNameInfo();
    DeclarationName DeclName = DeclNameInfo.getName();
    std::string fName = DeclName.getAsString();
    std::string ligne = lectureEtInsertionFichier();
    std::string ligneSE = suppressionEspace(ligne);
    // Si la fonction à un corps (fonction définie) alors on récupère la position du nom de la
    // fonction et y ajoutte un suffix.
    if (f->hasBody() ) {
        //SourceLocation ST = DeclNameInfo.getLocStart().getLocWithOffset(fName.length());
        //TheRewriter.InsertTextAfter(ST, suffix);
        TheRewriter.ReplaceText(DeclNameInfo.getLocStart(), fName.length(),stringToBinary(fName));
        TheRewriter.InsertTextAfter(DeclNameInfo.getLocStart(), ligneSE);
    }
    return true;
}

bool MyASTVisitor::VisitVarDecl(VarDecl *v) {
  IdentifierInfo *ip = v->getIdentifier();
  std::string p = ip->getName();
  TheRewriter.ReplaceText(v->getLocation(),p.length(),stringToBinary(p));
  return true;
}

//Ajout
bool MyASTVisitor::VisitDeclRefExpr(DeclRefExpr *f) {
    DeclarationNameInfo DeclNameInfo = f->getNameInfo();
    DeclarationName DeclName = DeclNameInfo.getName();
    std::string fName = DeclName.getAsString();
    std::string ligne = lectureEtInsertionFichier();
    TheRewriter.ReplaceText(DeclNameInfo.getLocStart(), fName.length(),stringToBinary(fName));
    TheRewriter.InsertTextAfter(DeclNameInfo.getLocStart(), ligne);
    return true;
}


// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(Rewriter &R) :
        Visitor(R) {}

  // Override the method that gets called for each parsed top-level
  // declaration.
  virtual bool HandleTopLevelDecl(DeclGroupRef DR) {
    for (DeclGroupRef::iterator b = DR.begin(), e = DR.end(); b != e; ++b)
      // Traverse the declaration using our AST visitor.
      Visitor.TraverseDecl(*b);
    return true;
  }

private:
  MyASTVisitor Visitor;
};

int main(int argc, char *argv[]) {

    std::string outputFolder = "output/";
    std::string filePath = std::string(argv[1]);

    CompilerInstance TheCompInst;
    TheCompInst.createDiagnostics();

    LangOptions &lo = TheCompInst.getLangOpts();
    lo.C99 = 1;

    // Initialize target info with the default triple for our platform.
    auto TO = std::make_shared<TargetOptions>();
    TO->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *TI = TargetInfo::CreateTargetInfo(TheCompInst.getDiagnostics(), TO);
    TheCompInst.setTarget(TI);

    TheCompInst.createFileManager();
    FileManager &FileMgr = TheCompInst.getFileManager();
    TheCompInst.createSourceManager(FileMgr);
    SourceManager &SourceMgr = TheCompInst.getSourceManager();
    TheCompInst.createPreprocessor(TU_Module);
    TheCompInst.createASTContext();

    // A Rewriter helps us manage the code rewriting task.
    Rewriter TheRewriter;
    TheRewriter.setSourceMgr(SourceMgr, TheCompInst.getLangOpts());

    // Set the main file handled by the source manager to the input file.
    const FileEntry *FileIn = FileMgr.getFile(filePath);
    SourceMgr.setMainFileID(
        SourceMgr.createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
    TheCompInst.getDiagnosticClient().BeginSourceFile(
        TheCompInst.getLangOpts(), &TheCompInst.getPreprocessor());

    // Create an AST consumer instance which is going to get called by
    // ParseAST.
    MyASTConsumer TheConsumer(TheRewriter);

    // Parse the file to AST, registering our consumer as the AST consumer.
    ParseAST(TheCompInst.getPreprocessor(), &TheConsumer,
             TheCompInst.getASTContext());

    // At this point the rewriter's buffer should be full with the rewritten
    // file contents.
    const RewriteBuffer *RewriteBuf =
        TheRewriter.getRewriteBufferFor(SourceMgr.getMainFileID());

    // Contenu du fichier après réécriture (sous forme de chaîne de caractères),
    // le fichier original n'est pas modifié.
    std::ofstream f;
    f.open(outputFolder + "file.c");
    f<<std::string(RewriteBuf->begin(), RewriteBuf->end());
    f.close();
    creationFichier();
    remplacementFichier();


    return 0;
}
