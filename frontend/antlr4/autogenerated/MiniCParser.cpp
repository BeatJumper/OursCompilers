
// Generated from MiniC.g4 by ANTLR 4.12.0


#include "MiniCVisitor.h"

#include "MiniCParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct MiniCParserStaticData final {
  MiniCParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  MiniCParserStaticData(const MiniCParserStaticData&) = delete;
  MiniCParserStaticData(MiniCParserStaticData&&) = delete;
  MiniCParserStaticData& operator=(const MiniCParserStaticData&) = delete;
  MiniCParserStaticData& operator=(MiniCParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag minicParserOnceFlag;
MiniCParserStaticData *minicParserStaticData = nullptr;

void minicParserInitialize() {
  assert(minicParserStaticData == nullptr);
  auto staticData = std::make_unique<MiniCParserStaticData>(
    std::vector<std::string>{
      "compileUnit", "funcDef", "block", "blockItemList", "blockItem", "varDecl", 
      "basicType", "varDef", "statement", "cond", "relExp", "relOp", "expr", 
      "addExp", "addOp", "unaryExp", "primaryExp", "realParamList", "lVal"
    },
    std::vector<std::string>{
      "", "'('", "')'", "';'", "'{'", "'}'", "'='", "','", "'+'", "'-'", 
      "'<'", "'>'", "'<='", "'>='", "'return'", "'int'", "'void'", "'if'", 
      "'else'", "'while'", "'break'", "'continue'"
    },
    std::vector<std::string>{
      "", "T_L_PAREN", "T_R_PAREN", "T_SEMICOLON", "T_L_BRACE", "T_R_BRACE", 
      "T_ASSIGN", "T_COMMA", "T_ADD", "T_SUB", "T_LT", "T_GT", "T_LE", "T_GE", 
      "T_RETURN", "T_INT", "T_VOID", "T_IF", "T_ELSE", "T_WHILE", "T_BREAK", 
      "T_CONTINUE", "T_ID", "T_DIGIT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,24,175,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,1,0,1,0,5,0,41,8,0,10,0,12,
  	0,44,9,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,2,3,2,56,8,2,1,2,1,2,1,
  	3,4,3,61,8,3,11,3,12,3,62,1,4,1,4,3,4,67,8,4,1,5,1,5,1,5,1,5,5,5,73,8,
  	5,10,5,12,5,76,9,5,1,5,1,5,1,6,1,6,1,7,1,7,1,7,3,7,85,8,7,1,8,1,8,1,8,
  	1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,3,8,98,8,8,1,8,1,8,1,8,1,8,1,8,1,8,1,
  	8,1,8,3,8,108,8,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,3,8,120,8,8,
  	1,9,1,9,1,10,1,10,1,10,1,10,5,10,128,8,10,10,10,12,10,131,9,10,1,11,1,
  	11,1,12,1,12,1,13,1,13,1,13,1,13,5,13,141,8,13,10,13,12,13,144,9,13,1,
  	14,1,14,1,15,1,15,1,15,1,15,3,15,152,8,15,1,15,3,15,155,8,15,1,16,1,16,
  	1,16,1,16,1,16,1,16,3,16,163,8,16,1,17,1,17,1,17,5,17,168,8,17,10,17,
  	12,17,171,9,17,1,18,1,18,1,18,0,0,19,0,2,4,6,8,10,12,14,16,18,20,22,24,
  	26,28,30,32,34,36,0,2,1,0,10,13,1,0,8,9,178,0,42,1,0,0,0,2,47,1,0,0,0,
  	4,53,1,0,0,0,6,60,1,0,0,0,8,66,1,0,0,0,10,68,1,0,0,0,12,79,1,0,0,0,14,
  	81,1,0,0,0,16,119,1,0,0,0,18,121,1,0,0,0,20,123,1,0,0,0,22,132,1,0,0,
  	0,24,134,1,0,0,0,26,136,1,0,0,0,28,145,1,0,0,0,30,154,1,0,0,0,32,162,
  	1,0,0,0,34,164,1,0,0,0,36,172,1,0,0,0,38,41,3,2,1,0,39,41,3,10,5,0,40,
  	38,1,0,0,0,40,39,1,0,0,0,41,44,1,0,0,0,42,40,1,0,0,0,42,43,1,0,0,0,43,
  	45,1,0,0,0,44,42,1,0,0,0,45,46,5,0,0,1,46,1,1,0,0,0,47,48,5,15,0,0,48,
  	49,5,22,0,0,49,50,5,1,0,0,50,51,5,2,0,0,51,52,3,4,2,0,52,3,1,0,0,0,53,
  	55,5,4,0,0,54,56,3,6,3,0,55,54,1,0,0,0,55,56,1,0,0,0,56,57,1,0,0,0,57,
  	58,5,5,0,0,58,5,1,0,0,0,59,61,3,8,4,0,60,59,1,0,0,0,61,62,1,0,0,0,62,
  	60,1,0,0,0,62,63,1,0,0,0,63,7,1,0,0,0,64,67,3,16,8,0,65,67,3,10,5,0,66,
  	64,1,0,0,0,66,65,1,0,0,0,67,9,1,0,0,0,68,69,3,12,6,0,69,74,3,14,7,0,70,
  	71,5,7,0,0,71,73,3,14,7,0,72,70,1,0,0,0,73,76,1,0,0,0,74,72,1,0,0,0,74,
  	75,1,0,0,0,75,77,1,0,0,0,76,74,1,0,0,0,77,78,5,3,0,0,78,11,1,0,0,0,79,
  	80,5,15,0,0,80,13,1,0,0,0,81,84,5,22,0,0,82,83,5,6,0,0,83,85,3,24,12,
  	0,84,82,1,0,0,0,84,85,1,0,0,0,85,15,1,0,0,0,86,87,5,14,0,0,87,88,3,24,
  	12,0,88,89,5,3,0,0,89,120,1,0,0,0,90,91,3,36,18,0,91,92,5,6,0,0,92,93,
  	3,24,12,0,93,94,5,3,0,0,94,120,1,0,0,0,95,120,3,4,2,0,96,98,3,24,12,0,
  	97,96,1,0,0,0,97,98,1,0,0,0,98,99,1,0,0,0,99,120,5,3,0,0,100,101,5,17,
  	0,0,101,102,5,1,0,0,102,103,3,18,9,0,103,104,5,2,0,0,104,107,3,16,8,0,
  	105,106,5,18,0,0,106,108,3,16,8,0,107,105,1,0,0,0,107,108,1,0,0,0,108,
  	120,1,0,0,0,109,110,5,19,0,0,110,111,5,1,0,0,111,112,3,18,9,0,112,113,
  	5,2,0,0,113,114,3,16,8,0,114,120,1,0,0,0,115,116,5,20,0,0,116,120,5,3,
  	0,0,117,118,5,21,0,0,118,120,5,3,0,0,119,86,1,0,0,0,119,90,1,0,0,0,119,
  	95,1,0,0,0,119,97,1,0,0,0,119,100,1,0,0,0,119,109,1,0,0,0,119,115,1,0,
  	0,0,119,117,1,0,0,0,120,17,1,0,0,0,121,122,3,20,10,0,122,19,1,0,0,0,123,
  	129,3,26,13,0,124,125,3,22,11,0,125,126,3,26,13,0,126,128,1,0,0,0,127,
  	124,1,0,0,0,128,131,1,0,0,0,129,127,1,0,0,0,129,130,1,0,0,0,130,21,1,
  	0,0,0,131,129,1,0,0,0,132,133,7,0,0,0,133,23,1,0,0,0,134,135,3,26,13,
  	0,135,25,1,0,0,0,136,142,3,30,15,0,137,138,3,28,14,0,138,139,3,30,15,
  	0,139,141,1,0,0,0,140,137,1,0,0,0,141,144,1,0,0,0,142,140,1,0,0,0,142,
  	143,1,0,0,0,143,27,1,0,0,0,144,142,1,0,0,0,145,146,7,1,0,0,146,29,1,0,
  	0,0,147,155,3,32,16,0,148,149,5,22,0,0,149,151,5,1,0,0,150,152,3,34,17,
  	0,151,150,1,0,0,0,151,152,1,0,0,0,152,153,1,0,0,0,153,155,5,2,0,0,154,
  	147,1,0,0,0,154,148,1,0,0,0,155,31,1,0,0,0,156,157,5,1,0,0,157,158,3,
  	24,12,0,158,159,5,2,0,0,159,163,1,0,0,0,160,163,5,23,0,0,161,163,3,36,
  	18,0,162,156,1,0,0,0,162,160,1,0,0,0,162,161,1,0,0,0,163,33,1,0,0,0,164,
  	169,3,24,12,0,165,166,5,7,0,0,166,168,3,24,12,0,167,165,1,0,0,0,168,171,
  	1,0,0,0,169,167,1,0,0,0,169,170,1,0,0,0,170,35,1,0,0,0,171,169,1,0,0,
  	0,172,173,5,22,0,0,173,37,1,0,0,0,16,40,42,55,62,66,74,84,97,107,119,
  	129,142,151,154,162,169
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  minicParserStaticData = staticData.release();
}

}

MiniCParser::MiniCParser(TokenStream *input) : MiniCParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

MiniCParser::MiniCParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  MiniCParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *minicParserStaticData->atn, minicParserStaticData->decisionToDFA, minicParserStaticData->sharedContextCache, options);
}

MiniCParser::~MiniCParser() {
  delete _interpreter;
}

const atn::ATN& MiniCParser::getATN() const {
  return *minicParserStaticData->atn;
}

std::string MiniCParser::getGrammarFileName() const {
  return "MiniC.g4";
}

const std::vector<std::string>& MiniCParser::getRuleNames() const {
  return minicParserStaticData->ruleNames;
}

const dfa::Vocabulary& MiniCParser::getVocabulary() const {
  return minicParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView MiniCParser::getSerializedATN() const {
  return minicParserStaticData->serializedATN;
}


//----------------- CompileUnitContext ------------------------------------------------------------------

MiniCParser::CompileUnitContext::CompileUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::CompileUnitContext::EOF() {
  return getToken(MiniCParser::EOF, 0);
}

std::vector<MiniCParser::FuncDefContext *> MiniCParser::CompileUnitContext::funcDef() {
  return getRuleContexts<MiniCParser::FuncDefContext>();
}

MiniCParser::FuncDefContext* MiniCParser::CompileUnitContext::funcDef(size_t i) {
  return getRuleContext<MiniCParser::FuncDefContext>(i);
}

std::vector<MiniCParser::VarDeclContext *> MiniCParser::CompileUnitContext::varDecl() {
  return getRuleContexts<MiniCParser::VarDeclContext>();
}

MiniCParser::VarDeclContext* MiniCParser::CompileUnitContext::varDecl(size_t i) {
  return getRuleContext<MiniCParser::VarDeclContext>(i);
}


size_t MiniCParser::CompileUnitContext::getRuleIndex() const {
  return MiniCParser::RuleCompileUnit;
}


std::any MiniCParser::CompileUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitCompileUnit(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::CompileUnitContext* MiniCParser::compileUnit() {
  CompileUnitContext *_localctx = _tracker.createInstance<CompileUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, MiniCParser::RuleCompileUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(42);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_INT) {
      setState(40);
      _errHandler->sync(this);
      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx)) {
      case 1: {
        setState(38);
        funcDef();
        break;
      }

      case 2: {
        setState(39);
        varDecl();
        break;
      }

      default:
        break;
      }
      setState(44);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(45);
    match(MiniCParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FuncDefContext ------------------------------------------------------------------

MiniCParser::FuncDefContext::FuncDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_INT() {
  return getToken(MiniCParser::T_INT, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

tree::TerminalNode* MiniCParser::FuncDefContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::BlockContext* MiniCParser::FuncDefContext::block() {
  return getRuleContext<MiniCParser::BlockContext>(0);
}


size_t MiniCParser::FuncDefContext::getRuleIndex() const {
  return MiniCParser::RuleFuncDef;
}


std::any MiniCParser::FuncDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitFuncDef(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::FuncDefContext* MiniCParser::funcDef() {
  FuncDefContext *_localctx = _tracker.createInstance<FuncDefContext>(_ctx, getState());
  enterRule(_localctx, 2, MiniCParser::RuleFuncDef);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(47);
    match(MiniCParser::T_INT);
    setState(48);
    match(MiniCParser::T_ID);
    setState(49);
    match(MiniCParser::T_L_PAREN);
    setState(50);
    match(MiniCParser::T_R_PAREN);
    setState(51);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

MiniCParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::BlockContext::T_L_BRACE() {
  return getToken(MiniCParser::T_L_BRACE, 0);
}

tree::TerminalNode* MiniCParser::BlockContext::T_R_BRACE() {
  return getToken(MiniCParser::T_R_BRACE, 0);
}

MiniCParser::BlockItemListContext* MiniCParser::BlockContext::blockItemList() {
  return getRuleContext<MiniCParser::BlockItemListContext>(0);
}


size_t MiniCParser::BlockContext::getRuleIndex() const {
  return MiniCParser::RuleBlock;
}


std::any MiniCParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockContext* MiniCParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 4, MiniCParser::RuleBlock);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(53);
    match(MiniCParser::T_L_BRACE);
    setState(55);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16433178) != 0)) {
      setState(54);
      blockItemList();
    }
    setState(57);
    match(MiniCParser::T_R_BRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemListContext ------------------------------------------------------------------

MiniCParser::BlockItemListContext::BlockItemListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::BlockItemContext *> MiniCParser::BlockItemListContext::blockItem() {
  return getRuleContexts<MiniCParser::BlockItemContext>();
}

MiniCParser::BlockItemContext* MiniCParser::BlockItemListContext::blockItem(size_t i) {
  return getRuleContext<MiniCParser::BlockItemContext>(i);
}


size_t MiniCParser::BlockItemListContext::getRuleIndex() const {
  return MiniCParser::RuleBlockItemList;
}


std::any MiniCParser::BlockItemListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockItemList(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockItemListContext* MiniCParser::blockItemList() {
  BlockItemListContext *_localctx = _tracker.createInstance<BlockItemListContext>(_ctx, getState());
  enterRule(_localctx, 6, MiniCParser::RuleBlockItemList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(60); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(59);
      blockItem();
      setState(62); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 16433178) != 0));
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockItemContext ------------------------------------------------------------------

MiniCParser::BlockItemContext::BlockItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::StatementContext* MiniCParser::BlockItemContext::statement() {
  return getRuleContext<MiniCParser::StatementContext>(0);
}

MiniCParser::VarDeclContext* MiniCParser::BlockItemContext::varDecl() {
  return getRuleContext<MiniCParser::VarDeclContext>(0);
}


size_t MiniCParser::BlockItemContext::getRuleIndex() const {
  return MiniCParser::RuleBlockItem;
}


std::any MiniCParser::BlockItemContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockItem(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BlockItemContext* MiniCParser::blockItem() {
  BlockItemContext *_localctx = _tracker.createInstance<BlockItemContext>(_ctx, getState());
  enterRule(_localctx, 8, MiniCParser::RuleBlockItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(66);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MiniCParser::T_L_PAREN:
      case MiniCParser::T_SEMICOLON:
      case MiniCParser::T_L_BRACE:
      case MiniCParser::T_RETURN:
      case MiniCParser::T_IF:
      case MiniCParser::T_WHILE:
      case MiniCParser::T_BREAK:
      case MiniCParser::T_CONTINUE:
      case MiniCParser::T_ID:
      case MiniCParser::T_DIGIT: {
        enterOuterAlt(_localctx, 1);
        setState(64);
        statement();
        break;
      }

      case MiniCParser::T_INT: {
        enterOuterAlt(_localctx, 2);
        setState(65);
        varDecl();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDeclContext ------------------------------------------------------------------

MiniCParser::VarDeclContext::VarDeclContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::BasicTypeContext* MiniCParser::VarDeclContext::basicType() {
  return getRuleContext<MiniCParser::BasicTypeContext>(0);
}

std::vector<MiniCParser::VarDefContext *> MiniCParser::VarDeclContext::varDef() {
  return getRuleContexts<MiniCParser::VarDefContext>();
}

MiniCParser::VarDefContext* MiniCParser::VarDeclContext::varDef(size_t i) {
  return getRuleContext<MiniCParser::VarDefContext>(i);
}

tree::TerminalNode* MiniCParser::VarDeclContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

std::vector<tree::TerminalNode *> MiniCParser::VarDeclContext::T_COMMA() {
  return getTokens(MiniCParser::T_COMMA);
}

tree::TerminalNode* MiniCParser::VarDeclContext::T_COMMA(size_t i) {
  return getToken(MiniCParser::T_COMMA, i);
}


size_t MiniCParser::VarDeclContext::getRuleIndex() const {
  return MiniCParser::RuleVarDecl;
}


std::any MiniCParser::VarDeclContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitVarDecl(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::VarDeclContext* MiniCParser::varDecl() {
  VarDeclContext *_localctx = _tracker.createInstance<VarDeclContext>(_ctx, getState());
  enterRule(_localctx, 10, MiniCParser::RuleVarDecl);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(68);
    basicType();
    setState(69);
    varDef();
    setState(74);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_COMMA) {
      setState(70);
      match(MiniCParser::T_COMMA);
      setState(71);
      varDef();
      setState(76);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(77);
    match(MiniCParser::T_SEMICOLON);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BasicTypeContext ------------------------------------------------------------------

MiniCParser::BasicTypeContext::BasicTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::BasicTypeContext::T_INT() {
  return getToken(MiniCParser::T_INT, 0);
}


size_t MiniCParser::BasicTypeContext::getRuleIndex() const {
  return MiniCParser::RuleBasicType;
}


std::any MiniCParser::BasicTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBasicType(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::BasicTypeContext* MiniCParser::basicType() {
  BasicTypeContext *_localctx = _tracker.createInstance<BasicTypeContext>(_ctx, getState());
  enterRule(_localctx, 12, MiniCParser::RuleBasicType);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(79);
    match(MiniCParser::T_INT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarDefContext ------------------------------------------------------------------

MiniCParser::VarDefContext::VarDefContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::VarDefContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

tree::TerminalNode* MiniCParser::VarDefContext::T_ASSIGN() {
  return getToken(MiniCParser::T_ASSIGN, 0);
}

MiniCParser::ExprContext* MiniCParser::VarDefContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}


size_t MiniCParser::VarDefContext::getRuleIndex() const {
  return MiniCParser::RuleVarDef;
}


std::any MiniCParser::VarDefContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitVarDef(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::VarDefContext* MiniCParser::varDef() {
  VarDefContext *_localctx = _tracker.createInstance<VarDefContext>(_ctx, getState());
  enterRule(_localctx, 14, MiniCParser::RuleVarDef);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(81);
    match(MiniCParser::T_ID);
    setState(84);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == MiniCParser::T_ASSIGN) {
      setState(82);
      match(MiniCParser::T_ASSIGN);
      setState(83);
      expr();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

MiniCParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t MiniCParser::StatementContext::getRuleIndex() const {
  return MiniCParser::RuleStatement;
}

void MiniCParser::StatementContext::copyFrom(StatementContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- WhileStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::WhileStatementContext::T_WHILE() {
  return getToken(MiniCParser::T_WHILE, 0);
}

tree::TerminalNode* MiniCParser::WhileStatementContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::CondContext* MiniCParser::WhileStatementContext::cond() {
  return getRuleContext<MiniCParser::CondContext>(0);
}

tree::TerminalNode* MiniCParser::WhileStatementContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::StatementContext* MiniCParser::WhileStatementContext::statement() {
  return getRuleContext<MiniCParser::StatementContext>(0);
}

MiniCParser::WhileStatementContext::WhileStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BlockStatementContext ------------------------------------------------------------------

MiniCParser::BlockContext* MiniCParser::BlockStatementContext::block() {
  return getRuleContext<MiniCParser::BlockContext>(0);
}

MiniCParser::BlockStatementContext::BlockStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::BlockStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBlockStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IfElseStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::IfElseStatementContext::T_IF() {
  return getToken(MiniCParser::T_IF, 0);
}

tree::TerminalNode* MiniCParser::IfElseStatementContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::CondContext* MiniCParser::IfElseStatementContext::cond() {
  return getRuleContext<MiniCParser::CondContext>(0);
}

tree::TerminalNode* MiniCParser::IfElseStatementContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

std::vector<MiniCParser::StatementContext *> MiniCParser::IfElseStatementContext::statement() {
  return getRuleContexts<MiniCParser::StatementContext>();
}

MiniCParser::StatementContext* MiniCParser::IfElseStatementContext::statement(size_t i) {
  return getRuleContext<MiniCParser::StatementContext>(i);
}

tree::TerminalNode* MiniCParser::IfElseStatementContext::T_ELSE() {
  return getToken(MiniCParser::T_ELSE, 0);
}

MiniCParser::IfElseStatementContext::IfElseStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::IfElseStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitIfElseStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AssignStatementContext ------------------------------------------------------------------

MiniCParser::LValContext* MiniCParser::AssignStatementContext::lVal() {
  return getRuleContext<MiniCParser::LValContext>(0);
}

tree::TerminalNode* MiniCParser::AssignStatementContext::T_ASSIGN() {
  return getToken(MiniCParser::T_ASSIGN, 0);
}

MiniCParser::ExprContext* MiniCParser::AssignStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::AssignStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::AssignStatementContext::AssignStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::AssignStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAssignStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- BreakStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::BreakStatementContext::T_BREAK() {
  return getToken(MiniCParser::T_BREAK, 0);
}

tree::TerminalNode* MiniCParser::BreakStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::BreakStatementContext::BreakStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ExpressionStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ExpressionStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ExprContext* MiniCParser::ExpressionStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

MiniCParser::ExpressionStatementContext::ExpressionStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ExpressionStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitExpressionStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ContinueStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ContinueStatementContext::T_CONTINUE() {
  return getToken(MiniCParser::T_CONTINUE, 0);
}

tree::TerminalNode* MiniCParser::ContinueStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ContinueStatementContext::ContinueStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ReturnStatementContext ------------------------------------------------------------------

tree::TerminalNode* MiniCParser::ReturnStatementContext::T_RETURN() {
  return getToken(MiniCParser::T_RETURN, 0);
}

MiniCParser::ExprContext* MiniCParser::ReturnStatementContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::ReturnStatementContext::T_SEMICOLON() {
  return getToken(MiniCParser::T_SEMICOLON, 0);
}

MiniCParser::ReturnStatementContext::ReturnStatementContext(StatementContext *ctx) { copyFrom(ctx); }


std::any MiniCParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}
MiniCParser::StatementContext* MiniCParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 16, MiniCParser::RuleStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(119);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 9, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<MiniCParser::ReturnStatementContext>(_localctx);
      enterOuterAlt(_localctx, 1);
      setState(86);
      match(MiniCParser::T_RETURN);
      setState(87);
      expr();
      setState(88);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<MiniCParser::AssignStatementContext>(_localctx);
      enterOuterAlt(_localctx, 2);
      setState(90);
      lVal();
      setState(91);
      match(MiniCParser::T_ASSIGN);
      setState(92);
      expr();
      setState(93);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<MiniCParser::BlockStatementContext>(_localctx);
      enterOuterAlt(_localctx, 3);
      setState(95);
      block();
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<MiniCParser::ExpressionStatementContext>(_localctx);
      enterOuterAlt(_localctx, 4);
      setState(97);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 12582914) != 0)) {
        setState(96);
        expr();
      }
      setState(99);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 5: {
      _localctx = _tracker.createInstance<MiniCParser::IfElseStatementContext>(_localctx);
      enterOuterAlt(_localctx, 5);
      setState(100);
      match(MiniCParser::T_IF);
      setState(101);
      match(MiniCParser::T_L_PAREN);
      setState(102);
      cond();
      setState(103);
      match(MiniCParser::T_R_PAREN);
      setState(104);
      statement();
      setState(107);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 8, _ctx)) {
      case 1: {
        setState(105);
        match(MiniCParser::T_ELSE);
        setState(106);
        statement();
        break;
      }

      default:
        break;
      }
      break;
    }

    case 6: {
      _localctx = _tracker.createInstance<MiniCParser::WhileStatementContext>(_localctx);
      enterOuterAlt(_localctx, 6);
      setState(109);
      match(MiniCParser::T_WHILE);
      setState(110);
      match(MiniCParser::T_L_PAREN);
      setState(111);
      cond();
      setState(112);
      match(MiniCParser::T_R_PAREN);
      setState(113);
      statement();
      break;
    }

    case 7: {
      _localctx = _tracker.createInstance<MiniCParser::BreakStatementContext>(_localctx);
      enterOuterAlt(_localctx, 7);
      setState(115);
      match(MiniCParser::T_BREAK);
      setState(116);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    case 8: {
      _localctx = _tracker.createInstance<MiniCParser::ContinueStatementContext>(_localctx);
      enterOuterAlt(_localctx, 8);
      setState(117);
      match(MiniCParser::T_CONTINUE);
      setState(118);
      match(MiniCParser::T_SEMICOLON);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CondContext ------------------------------------------------------------------

MiniCParser::CondContext::CondContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::RelExpContext* MiniCParser::CondContext::relExp() {
  return getRuleContext<MiniCParser::RelExpContext>(0);
}


size_t MiniCParser::CondContext::getRuleIndex() const {
  return MiniCParser::RuleCond;
}


std::any MiniCParser::CondContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitCond(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::CondContext* MiniCParser::cond() {
  CondContext *_localctx = _tracker.createInstance<CondContext>(_ctx, getState());
  enterRule(_localctx, 18, MiniCParser::RuleCond);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(121);
    relExp();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelExpContext ------------------------------------------------------------------

MiniCParser::RelExpContext::RelExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::AddExpContext *> MiniCParser::RelExpContext::addExp() {
  return getRuleContexts<MiniCParser::AddExpContext>();
}

MiniCParser::AddExpContext* MiniCParser::RelExpContext::addExp(size_t i) {
  return getRuleContext<MiniCParser::AddExpContext>(i);
}

std::vector<MiniCParser::RelOpContext *> MiniCParser::RelExpContext::relOp() {
  return getRuleContexts<MiniCParser::RelOpContext>();
}

MiniCParser::RelOpContext* MiniCParser::RelExpContext::relOp(size_t i) {
  return getRuleContext<MiniCParser::RelOpContext>(i);
}


size_t MiniCParser::RelExpContext::getRuleIndex() const {
  return MiniCParser::RuleRelExp;
}


std::any MiniCParser::RelExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitRelExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::RelExpContext* MiniCParser::relExp() {
  RelExpContext *_localctx = _tracker.createInstance<RelExpContext>(_ctx, getState());
  enterRule(_localctx, 20, MiniCParser::RuleRelExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(123);
    addExp();
    setState(129);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15360) != 0)) {
      setState(124);
      relOp();
      setState(125);
      addExp();
      setState(131);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RelOpContext ------------------------------------------------------------------

MiniCParser::RelOpContext::RelOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::RelOpContext::T_LT() {
  return getToken(MiniCParser::T_LT, 0);
}

tree::TerminalNode* MiniCParser::RelOpContext::T_GT() {
  return getToken(MiniCParser::T_GT, 0);
}

tree::TerminalNode* MiniCParser::RelOpContext::T_LE() {
  return getToken(MiniCParser::T_LE, 0);
}

tree::TerminalNode* MiniCParser::RelOpContext::T_GE() {
  return getToken(MiniCParser::T_GE, 0);
}


size_t MiniCParser::RelOpContext::getRuleIndex() const {
  return MiniCParser::RuleRelOp;
}


std::any MiniCParser::RelOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitRelOp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::RelOpContext* MiniCParser::relOp() {
  RelOpContext *_localctx = _tracker.createInstance<RelOpContext>(_ctx, getState());
  enterRule(_localctx, 22, MiniCParser::RuleRelOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(132);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 15360) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExprContext ------------------------------------------------------------------

MiniCParser::ExprContext::ExprContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::AddExpContext* MiniCParser::ExprContext::addExp() {
  return getRuleContext<MiniCParser::AddExpContext>(0);
}


size_t MiniCParser::ExprContext::getRuleIndex() const {
  return MiniCParser::RuleExpr;
}


std::any MiniCParser::ExprContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitExpr(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::ExprContext* MiniCParser::expr() {
  ExprContext *_localctx = _tracker.createInstance<ExprContext>(_ctx, getState());
  enterRule(_localctx, 24, MiniCParser::RuleExpr);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(134);
    addExp();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddExpContext ------------------------------------------------------------------

MiniCParser::AddExpContext::AddExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::UnaryExpContext *> MiniCParser::AddExpContext::unaryExp() {
  return getRuleContexts<MiniCParser::UnaryExpContext>();
}

MiniCParser::UnaryExpContext* MiniCParser::AddExpContext::unaryExp(size_t i) {
  return getRuleContext<MiniCParser::UnaryExpContext>(i);
}

std::vector<MiniCParser::AddOpContext *> MiniCParser::AddExpContext::addOp() {
  return getRuleContexts<MiniCParser::AddOpContext>();
}

MiniCParser::AddOpContext* MiniCParser::AddExpContext::addOp(size_t i) {
  return getRuleContext<MiniCParser::AddOpContext>(i);
}


size_t MiniCParser::AddExpContext::getRuleIndex() const {
  return MiniCParser::RuleAddExp;
}


std::any MiniCParser::AddExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAddExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::AddExpContext* MiniCParser::addExp() {
  AddExpContext *_localctx = _tracker.createInstance<AddExpContext>(_ctx, getState());
  enterRule(_localctx, 26, MiniCParser::RuleAddExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(136);
    unaryExp();
    setState(142);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_ADD

    || _la == MiniCParser::T_SUB) {
      setState(137);
      addOp();
      setState(138);
      unaryExp();
      setState(144);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AddOpContext ------------------------------------------------------------------

MiniCParser::AddOpContext::AddOpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::AddOpContext::T_ADD() {
  return getToken(MiniCParser::T_ADD, 0);
}

tree::TerminalNode* MiniCParser::AddOpContext::T_SUB() {
  return getToken(MiniCParser::T_SUB, 0);
}


size_t MiniCParser::AddOpContext::getRuleIndex() const {
  return MiniCParser::RuleAddOp;
}


std::any MiniCParser::AddOpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitAddOp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::AddOpContext* MiniCParser::addOp() {
  AddOpContext *_localctx = _tracker.createInstance<AddOpContext>(_ctx, getState());
  enterRule(_localctx, 28, MiniCParser::RuleAddOp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(145);
    _la = _input->LA(1);
    if (!(_la == MiniCParser::T_ADD

    || _la == MiniCParser::T_SUB)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnaryExpContext ------------------------------------------------------------------

MiniCParser::UnaryExpContext::UnaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

MiniCParser::PrimaryExpContext* MiniCParser::UnaryExpContext::primaryExp() {
  return getRuleContext<MiniCParser::PrimaryExpContext>(0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

tree::TerminalNode* MiniCParser::UnaryExpContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

MiniCParser::RealParamListContext* MiniCParser::UnaryExpContext::realParamList() {
  return getRuleContext<MiniCParser::RealParamListContext>(0);
}


size_t MiniCParser::UnaryExpContext::getRuleIndex() const {
  return MiniCParser::RuleUnaryExp;
}


std::any MiniCParser::UnaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitUnaryExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::UnaryExpContext* MiniCParser::unaryExp() {
  UnaryExpContext *_localctx = _tracker.createInstance<UnaryExpContext>(_ctx, getState());
  enterRule(_localctx, 30, MiniCParser::RuleUnaryExp);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(154);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(147);
      primaryExp();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(148);
      match(MiniCParser::T_ID);
      setState(149);
      match(MiniCParser::T_L_PAREN);
      setState(151);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 12582914) != 0)) {
        setState(150);
        realParamList();
      }
      setState(153);
      match(MiniCParser::T_R_PAREN);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrimaryExpContext ------------------------------------------------------------------

MiniCParser::PrimaryExpContext::PrimaryExpContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_L_PAREN() {
  return getToken(MiniCParser::T_L_PAREN, 0);
}

MiniCParser::ExprContext* MiniCParser::PrimaryExpContext::expr() {
  return getRuleContext<MiniCParser::ExprContext>(0);
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_R_PAREN() {
  return getToken(MiniCParser::T_R_PAREN, 0);
}

tree::TerminalNode* MiniCParser::PrimaryExpContext::T_DIGIT() {
  return getToken(MiniCParser::T_DIGIT, 0);
}

MiniCParser::LValContext* MiniCParser::PrimaryExpContext::lVal() {
  return getRuleContext<MiniCParser::LValContext>(0);
}


size_t MiniCParser::PrimaryExpContext::getRuleIndex() const {
  return MiniCParser::RulePrimaryExp;
}


std::any MiniCParser::PrimaryExpContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitPrimaryExp(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::PrimaryExpContext* MiniCParser::primaryExp() {
  PrimaryExpContext *_localctx = _tracker.createInstance<PrimaryExpContext>(_ctx, getState());
  enterRule(_localctx, 32, MiniCParser::RulePrimaryExp);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(162);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case MiniCParser::T_L_PAREN: {
        enterOuterAlt(_localctx, 1);
        setState(156);
        match(MiniCParser::T_L_PAREN);
        setState(157);
        expr();
        setState(158);
        match(MiniCParser::T_R_PAREN);
        break;
      }

      case MiniCParser::T_DIGIT: {
        enterOuterAlt(_localctx, 2);
        setState(160);
        match(MiniCParser::T_DIGIT);
        break;
      }

      case MiniCParser::T_ID: {
        enterOuterAlt(_localctx, 3);
        setState(161);
        lVal();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RealParamListContext ------------------------------------------------------------------

MiniCParser::RealParamListContext::RealParamListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<MiniCParser::ExprContext *> MiniCParser::RealParamListContext::expr() {
  return getRuleContexts<MiniCParser::ExprContext>();
}

MiniCParser::ExprContext* MiniCParser::RealParamListContext::expr(size_t i) {
  return getRuleContext<MiniCParser::ExprContext>(i);
}

std::vector<tree::TerminalNode *> MiniCParser::RealParamListContext::T_COMMA() {
  return getTokens(MiniCParser::T_COMMA);
}

tree::TerminalNode* MiniCParser::RealParamListContext::T_COMMA(size_t i) {
  return getToken(MiniCParser::T_COMMA, i);
}


size_t MiniCParser::RealParamListContext::getRuleIndex() const {
  return MiniCParser::RuleRealParamList;
}


std::any MiniCParser::RealParamListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitRealParamList(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::RealParamListContext* MiniCParser::realParamList() {
  RealParamListContext *_localctx = _tracker.createInstance<RealParamListContext>(_ctx, getState());
  enterRule(_localctx, 34, MiniCParser::RuleRealParamList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(164);
    expr();
    setState(169);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == MiniCParser::T_COMMA) {
      setState(165);
      match(MiniCParser::T_COMMA);
      setState(166);
      expr();
      setState(171);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LValContext ------------------------------------------------------------------

MiniCParser::LValContext::LValContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* MiniCParser::LValContext::T_ID() {
  return getToken(MiniCParser::T_ID, 0);
}


size_t MiniCParser::LValContext::getRuleIndex() const {
  return MiniCParser::RuleLVal;
}


std::any MiniCParser::LValContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<MiniCVisitor*>(visitor))
    return parserVisitor->visitLVal(this);
  else
    return visitor->visitChildren(this);
}

MiniCParser::LValContext* MiniCParser::lVal() {
  LValContext *_localctx = _tracker.createInstance<LValContext>(_ctx, getState());
  enterRule(_localctx, 36, MiniCParser::RuleLVal);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(172);
    match(MiniCParser::T_ID);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void MiniCParser::initialize() {
  ::antlr4::internal::call_once(minicParserOnceFlag, minicParserInitialize);
}
