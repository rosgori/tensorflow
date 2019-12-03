//===- Translation.cpp - Translation registry -----------------------------===//
//
// Copyright 2019 The MLIR Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// =============================================================================
//
// Definitions of the translation registry.
//
//===----------------------------------------------------------------------===//

#include "mlir/Translation.h"
#include "mlir/IR/Module.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "llvm/Support/SourceMgr.h"

using namespace mlir;

// Get the mutable static map between registered "to MLIR" translations and the
// TranslateToMLIRFunctions that perform those translations.
static llvm::StringMap<TranslateSourceMgrToMLIRFunction> &
getMutableTranslationToMLIRRegistry() {
  static llvm::StringMap<TranslateSourceMgrToMLIRFunction>
      translationToMLIRRegistry;
  return translationToMLIRRegistry;
}
// Get the mutable static map between registered "from MLIR" translations and
// the TranslateFromMLIRFunctions that perform those translations.
static llvm::StringMap<TranslateFromMLIRFunction> &
getMutableTranslationFromMLIRRegistry() {
  static llvm::StringMap<TranslateFromMLIRFunction> translationFromMLIRRegistry;
  return translationFromMLIRRegistry;
}

// Get the mutable static map between registered file-to-file MLIR translations
// and the TranslateFunctions that perform those translations.
static llvm::StringMap<TranslateFunction> &getMutableTranslationRegistry() {
  static llvm::StringMap<TranslateFunction> translationRegistry;
  return translationRegistry;
}

// Puts `function` into the to-MLIR translation registry unless there is already
// a function registered for the same name.
static void registerTranslateToMLIRFunction(
    StringRef name, const TranslateSourceMgrToMLIRFunction &function) {
  auto &translationToMLIRRegistry = getMutableTranslationToMLIRRegistry();
  if (translationToMLIRRegistry.find(name) != translationToMLIRRegistry.end())
    llvm::report_fatal_error(
        "Attempting to overwrite an existing <to> function");
  assert(function && "Attempting to register an empty translate <to> function");
  translationToMLIRRegistry[name] = function;
}

TranslateToMLIRRegistration::TranslateToMLIRRegistration(
    StringRef name, const TranslateSourceMgrToMLIRFunction &function) {
  registerTranslateToMLIRFunction(name, function);
}

// Wraps `function` with a lambda that extracts a StringRef from a source
// manager and registers the wrapper lambda as a to-MLIR conversion.
TranslateToMLIRRegistration::TranslateToMLIRRegistration(
    StringRef name, const TranslateStringRefToMLIRFunction &function) {
  auto translationFunction = [function](llvm::SourceMgr &sourceMgr,
                                        MLIRContext *ctx) {
    const llvm::MemoryBuffer *buffer =
        sourceMgr.getMemoryBuffer(sourceMgr.getMainFileID());
    return function(buffer->getBuffer(), ctx);
  };
  registerTranslateToMLIRFunction(name, translationFunction);
}

TranslateFromMLIRRegistration::TranslateFromMLIRRegistration(
    StringRef name, const TranslateFromMLIRFunction &function) {
  auto &translationFromMLIRRegistry = getMutableTranslationFromMLIRRegistry();
  if (translationFromMLIRRegistry.find(name) !=
      translationFromMLIRRegistry.end())
    llvm::report_fatal_error(
        "Attempting to overwrite an existing <from> function");
  assert(function &&
         "Attempting to register an empty translate <from> function");
  translationFromMLIRRegistry[name] = function;
}

TranslateRegistration::TranslateRegistration(
    StringRef name, const TranslateFunction &function) {
  auto &translationRegistry = getMutableTranslationRegistry();
  if (translationRegistry.find(name) != translationRegistry.end())
    llvm::report_fatal_error(
        "Attempting to overwrite an existing <file-to-file> function");
  assert(function &&
         "Attempting to register an empty translate <file-to-file> function");
  translationRegistry[name] = function;
}

// Merely add the const qualifier to the mutable registry so that external users
// cannot modify it.
const llvm::StringMap<TranslateSourceMgrToMLIRFunction> &
mlir::getTranslationToMLIRRegistry() {
  return getMutableTranslationToMLIRRegistry();
}

const llvm::StringMap<TranslateFromMLIRFunction> &
mlir::getTranslationFromMLIRRegistry() {
  return getMutableTranslationFromMLIRRegistry();
}

const llvm::StringMap<TranslateFunction> &mlir::getTranslationRegistry() {
  return getMutableTranslationRegistry();
}
