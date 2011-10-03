{
  Copyright 2001-2011 Michalis Kamburelis.

  This file is part of "Castle Game Engine".

  "Castle Game Engine" is free software; see the file COPYING.txt,
  included in this distribution, for details about the copyright.

  "Castle Game Engine" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ----------------------------------------------------------------------------
}

(*
  @abstract(Parser for CastleScript language, see
  [http://castle-engine.sourceforge.net/kambi_script.php].)

  Can parse whole program in CastleScript language, is also prepared
  to parse only a single expression (usefull for cases when I need
  to input only a mathematical expression, like for glplotter function
  expression).
*)

unit CastleScriptParser;

interface

uses CastleScript, CastleScriptLexer, Math;

type
  { Reexported in this unit, so that the identifier EKamScriptSyntaxError
    will be visible when using this unit. }
  EKamScriptSyntaxError = CastleScriptLexer.EKamScriptSyntaxError;

{ Creates and returns instance of TKamScriptExpression,
  that represents parsed tree of expression in S.

  This parses a subset of CastleScript language, that allows you
  to define only one expression without any assignments.
  Also the end result is always casted to the float() type
  (just like it would be wrapped inside float() function call ---
  in fact this is exactly what happens.)

  The end result is that this is perfect for describing things
  like function expressions, ideal e.g. for
  [http://castle-engine.sourceforge.net/glplotter_and_gen_function.php].

  @param(Variables contains a list of named values you want
    to allow in this expression.

    Important: They will all have
    OwnedByParentExpression set to @false, and you will have to
    free them yourself.
    That's because given expression may use the same variable more than once
    (so freeing it twice would cause bugs), or not use it at all
    (so it will be automatically freed at all).

    So setting OwnedByParentExpression and freeing it yourself
    is the only sensible thing to do.)

  @raises(EKamScriptSyntaxError in case of error when parsing expression.) }
function ParseFloatExpression(const S: string;
  const Variables: array of TKamScriptValue): TKamScriptExpression;

{ Parse constant float expression.
  This can be used as a great replacement for StrToFloat.
  Takes a string with any constant mathematical expression,
  according to CastleScript syntax, parses it and calculates.

  @raises(EKamScriptSyntaxError in case of error when parsing expression.) }
function ParseConstantFloatExpression(const S: string): Float;

{ Parse CastleScript program.

  Variable list works like for ParseFloatExpression, see there for
  description.

  @raises(EKamScriptSyntaxError in case of error when parsing expression.)

  @groupBegin }
function ParseProgram(const S: string;
  const Variables: array of TKamScriptValue): TKamScriptProgram; overload;
function ParseProgram(const S: string;
  const Variables: TKamScriptValueList): TKamScriptProgram; overload;
{ @groupEnd }

implementation

uses SysUtils, CastleScriptCoreFunctions;

function Expression(
  const Lexer: TKamScriptLexer;
  Environment: TKamScriptEnvironment;
  const Variables: array of TKamScriptValue): TKamScriptExpression; forward;

function NonAssignmentExpression(
  const Lexer: TKamScriptLexer;
  Environment: TKamScriptEnvironment;
  const AllowFullExpressionInFactor: boolean;
  const Variables: array of TKamScriptValue): TKamScriptExpression;

  function BinaryOper(tok: TToken): TKamScriptFunctionClass;
  begin
    case tok of
      tokPlus: Result := TKamScriptAdd;
      tokMinus: Result := TKamScriptSubtract;

      tokMultiply: Result := TKamScriptMultiply;
      tokDivide: Result := TKamScriptDivide;
      tokPower: Result := TKamScriptPower;
      tokModulo: Result := TKamScriptModulo;

      tokGreater: Result := TKamScriptGreater;
      tokLesser: Result := TKamScriptLesser;
      tokGreaterEqual: Result := TKamScriptGreaterEq;
      tokLesserEqual: Result := TKamScriptLesserEq;
      tokEqual: Result := TKamScriptEqual;
      tokNotEqual: Result := TKamScriptNotEqual;

      else raise EKamScriptParserError.Create(Lexer,
        'internal error : token not a binary operator');
    end
  end;

const
  SErrWrongFactor = 'wrong factor (expected identifier, constant, "-", "(" or function name)';
  SErrOperRelacExpected = 'comparison operator (>, <, >=, <=, = or <>) expected';

  FactorOperator = [tokMultiply, tokDivide, tokPower, tokModulo];
  TermOperator = [tokPlus, tokMinus];
  ComparisonOperator = [tokGreater, tokLesser, tokGreaterEqual, tokLesserEqual, tokEqual, tokNotEqual];

  function Operand: TKamScriptValue;
  var
    I: Integer;
  begin
    Lexer.CheckTokenIs(tokIdentifier);

    Result := nil;
    for I := 0 to Length(Variables) - 1 do
      if SameText(Variables[I].Name, Lexer.TokenString) then
      begin
        Result := Variables[I];
        Break;
      end;

    if Result = nil then
      raise EKamScriptParserError.CreateFmt(Lexer, 'Undefined identifier "%s"',
        [Lexer.TokenString]);

    Lexer.NextToken;
  end;

  { Returns either Expression or NonAssignmentExpression, depending on
    AllowFullExpressionInFactor value. }
  function ExpressionInsideFactor: TKamScriptExpression;
  begin
    if AllowFullExpressionInFactor then
      Result := Expression(Lexer, Environment, Variables) else
      Result := NonAssignmentExpression(Lexer, Environment,
        AllowFullExpressionInFactor, Variables);
  end;

  function Factor: TKamScriptExpression;
  var
    FC: TKamScriptFunctionClass;
    FParams: TKamScriptExpressionList;
  begin
    Result := nil;
    try
      case Lexer.Token of
        tokIdentifier: Result := Operand;
        tokInteger: begin
            Result := TKamScriptInteger.Create(false, Lexer.TokenInteger);
            Result.Environment := Environment;
            Lexer.NextToken;
          end;
        tokFloat: begin
            Result := TKamScriptFloat.Create(false, Lexer.TokenFloat);
            Result.Environment := Environment;
            Lexer.NextToken;
          end;
        tokBoolean: begin
            Result := TKamScriptBoolean.Create(false, Lexer.TokenBoolean);
            Result.Environment := Environment;
            Lexer.NextToken;
          end;
        tokString: begin
            Result := TKamScriptString.Create(false, Lexer.TokenString);
            Result.Environment := Environment;
            Lexer.NextToken;
          end;
        tokMinus: begin
            Lexer.NextToken;
            Result := TKamScriptNegate.Create([Factor()]);
            Result.Environment := Environment;
          end;
        tokLParen: begin
            Lexer.NextToken;
            Result := ExpressionInsideFactor;
            Lexer.CheckTokenIs(tokRParen);
            Lexer.NextToken;
          end;
        tokFuncName: begin
            FC := Lexer.TokenFunctionClass;
            Lexer.NextToken;
            FParams := TKamScriptExpressionList.Create(false);
            try
              try
                Lexer.CheckTokenIs(tokLParen);
                Lexer.NextToken;

                if Lexer.Token <> tokRParen then
                begin
                  repeat
                    FParams.Add(ExpressionInsideFactor);
                    if Lexer.Token = tokRParen then
                      Break;
                    Lexer.CheckTokenIs(tokComma);
                    Lexer.NextToken;
                  until false;
                end;

                Lexer.CheckTokenIs(tokRParen);
                Lexer.NextToken;
              except FParams.FreeContentsByParentExpression; raise; end;
              Result := FC.Create(FParams);
              Result.Environment := Environment;
            finally FParams.Free end;
          end;
        else raise EKamScriptParserError.Create(Lexer, SErrWrongFactor +
          ', but got "' + Lexer.TokenDescription + '"');
      end;
    except Result.FreeByParentExpression; raise end;
  end;

  function Term: TKamScriptExpression;
  var
    FC: TKamScriptFunctionClass;
  begin
    Result := nil;
    try
      Result := Factor;
      while Lexer.Token in FactorOperator do
      begin
        FC := BinaryOper(Lexer.Token);
        Lexer.NextToken;
        Result := FC.Create([Result, Factor]);
        Result.Environment := Environment;
      end;
    except Result.FreeByParentExpression; raise end;
  end;

  function ComparisonArgument: TKamScriptExpression;
  var
    FC: TKamScriptFunctionClass;
  begin
    Result := nil;
    try
      Result := Term;
      while Lexer.Token in TermOperator do
      begin
        FC := BinaryOper(Lexer.Token);
        Lexer.NextToken;
        Result := FC.Create([Result, Term]);
        Result.Environment := Environment;
      end;
    except Result.FreeByParentExpression; raise end;
  end;

var
  FC: TKamScriptFunctionClass;
begin
  Result := nil;
  try
    Result := ComparisonArgument;
    while Lexer.Token in ComparisonOperator do
    begin
      FC := BinaryOper(Lexer.Token);
      Lexer.NextToken;
      Result := FC.Create([Result, ComparisonArgument]);
      Result.Environment := Environment;
    end;
  except Result.FreeByParentExpression; raise end;
end;

type
  TKamScriptValuesArray = array of TKamScriptValue;

function VariablesListToArray(
  const Variables: TKamScriptValueList): TKamScriptValuesArray;
var
  I: Integer;
begin
  SetLength(Result, Variables.Count);
  for I := 0 to Variables.Count - 1 do
    Result[I] := Variables[I];
end;

function Expression(
  const Lexer: TKamScriptLexer;
  Environment: TKamScriptEnvironment;
  const Variables: TKamScriptValueList): TKamScriptExpression;
begin
  Result := Expression(Lexer, Environment, VariablesListToArray(Variables));
end;

function Expression(
  const Lexer: TKamScriptLexer;
  Environment: TKamScriptEnvironment;
  const Variables: array of TKamScriptValue): TKamScriptExpression;

  function PossiblyAssignmentExpression: TKamScriptExpression;
  { How to parse this?

    Straighforward approach is to try parsing
    Operand, then check is it followed by ":=".
    In case of parsing errors (we can catch them by EKamScriptParserError),
    or something else than ":=", we rollback and parse NonAssignmentExpression.

    The trouble with this approach: "rollback". This is uneasy,
    as you have to carefully remember all tokens eaten during
    Operand parsing, and unget them to lexer (or otherwise reparse them).

    Simpler and faster approach used: just always parse an
    NonAssignmentExpression. This uses the fact that Operand is
    also a valid NonAssignmentExpression, and NonAssignmentExpression
    will not eat anything after ":=" (following the grammar, ":="
    cannot occur within NonAssignmentExpression without parenthesis).
    After parsing NonAssignmentExpression, we can check for ":=". }
  var
    Operand, AssignedValue: TKamScriptExpression;
  begin
    Result := NonAssignmentExpression(Lexer, Environment, true, Variables);
    try
      if Lexer.Token = tokAssignment then
      begin
        Lexer.NextToken;

        AssignedValue := PossiblyAssignmentExpression();

        Operand := Result;
        { set Result to nil, in case of exception from TKamScriptAssignment
          constructor. }
        Result := nil;

        { TKamScriptAssignment in constructor checks that
          Operand is actually a simple writeable operand. }
        Result := TKamScriptAssignment.Create([Operand, AssignedValue]);
        Result.Environment := Environment;
      end;
    except Result.FreeByParentExpression; raise end;
  end;

var
  SequenceArgs: TKamScriptExpressionList;
begin
  Result := nil;
  try
    Result := PossiblyAssignmentExpression;

    if Lexer.Token = tokSemicolon then
    begin
      SequenceArgs := TKamScriptExpressionList.Create(false);
      try
        try
          SequenceArgs.Add(Result);
          Result := nil;

          while Lexer.Token = tokSemicolon do
          begin
            Lexer.NextToken;
            SequenceArgs.Add(PossiblyAssignmentExpression);
          end;
        except SequenceArgs.FreeContentsByParentExpression; raise end;

        Result := TKamScriptSequence.Create(SequenceArgs);
        Result.Environment := Environment;
      finally FreeAndNil(SequenceArgs) end;
    end;
  except Result.FreeByParentExpression; raise end;
end;

function AProgram(
  const Lexer: TKamScriptLexer;
  const GlobalVariables: array of TKamScriptValue): TKamScriptProgram;
var
  Environment: TKamScriptEnvironment;

  function AFunction: TKamScriptFunctionDefinition;
  var
    BodyVariables: TKamScriptValueList;
    Parameter: TKamScriptValue;
  begin
    Result := TKamScriptFunctionDefinition.Create;
    try
      Lexer.CheckTokenIs(tokIdentifier);
      Result.Name := Lexer.TokenString;
      Lexer.NextToken;

      BodyVariables := TKamScriptValueList.Create(false);
      try
        Lexer.CheckTokenIs(tokLParen);
        Lexer.NextToken;

        if Lexer.Token <> tokRParen then
        begin
          repeat
            Lexer.CheckTokenIs(tokIdentifier);
            Parameter := TKamScriptParameterValue.Create(true);
            Parameter.Environment := Environment;
            Parameter.Name := Lexer.TokenString;
            Parameter.OwnedByParentExpression := false;
            Result.Parameters.Add(Parameter);
            BodyVariables.Add(Parameter);
            Lexer.NextToken;

            if Lexer.Token = tokRParen then
              Break else
              begin
                Lexer.CheckTokenIs(tokComma);
                Lexer.NextToken;
              end;
          until false;
        end;

        Lexer.NextToken; { eat ")" }

        { We first added parameters, then added GlobalVariables,
          so when resolving, parameter names will hide global
          variable names, just like they should in normal language. }
        BodyVariables.AddArray(GlobalVariables);

        Result.Body := Expression(Lexer, Environment, BodyVariables);
      finally FreeAndNil(BodyVariables); end;
    except FreeAndNil(Result); raise end;
  end;

begin
  Result := TKamScriptProgram.Create;
  try
    Environment := Result.Environment;
    while Lexer.Token = tokFunctionKeyword do
    begin
      Lexer.NextToken;
      Result.Functions.Add(AFunction);
    end;
  except FreeAndNil(Result); raise end;
end;

{ ParseFloatExpression ------------------------------------------------------- }

function ParseFloatExpression(const S: string;
  const Variables: array of TKamScriptValue): TKamScriptExpression;
var
  Lexer: TKamScriptLexer;
  I: Integer;
begin
  for I := 0 to Length(Variables) - 1 do
    Variables[I].OwnedByParentExpression := false;

  Lexer := TKamScriptLexer.Create(s);
  try
    Result := nil;
    try
      try
        Result := NonAssignmentExpression(Lexer, nil { no Environment }, false, Variables);
        Lexer.CheckTokenIs(tokEnd);
      except
        { Change EKamScriptFunctionArgumentsError (raised when
          creating functions) to EKamScriptParserError.
          This allows the caller to catch only EKamScriptSyntaxError,
          and adds position information to error message. }
        on E: EKamScriptFunctionArgumentsError do
          raise EKamScriptParserError.Create(Lexer, E.Message);
      end;

      { At the end, wrap Result in float() cast. }
      Result := TKamScriptFloatFun.Create([Result]);
    except Result.FreeByParentExpression; raise end;
  finally Lexer.Free end;
end;

{ ParseConstantFloatExpression ----------------------------------------------- }

function ParseConstantFloatExpression(const S: string): Float;
var
  Expr: TKamScriptExpression;
begin
  try
    Expr := ParseFloatExpression(s, []);
  except
    on E: EKamScriptSyntaxError do
    begin
      E.Message := 'Error when parsing constant expression: ' + E.Message;
      raise;
    end;
  end;

  try
    Result := (Expr.Execute as TKamScriptFloat).Value;
  finally Expr.Free end;
end;

{ ParseProgram --------------------------------------------------------------- }

function ParseProgram(const S: string;
  const Variables: TKamScriptValueList): TKamScriptProgram;
begin
  Result := ParseProgram(S, VariablesListToArray(Variables));
end;

function ParseProgram(const S: string;
  const Variables: array of TKamScriptValue): TKamScriptProgram;
var
  Lexer: TKamScriptLexer;
  I: Integer;
begin
  for I := 0 to Length(Variables) - 1 do
    Variables[I].OwnedByParentExpression := false;

  Lexer := TKamScriptLexer.Create(s);
  try
    Result := nil;
    try
      try
        Result := AProgram(Lexer, Variables);
        Lexer.CheckTokenIs(tokEnd);
      except
        { Change EKamScriptFunctionArgumentsError (raised when
          creating functions) to EKamScriptParserError.
          This allows the caller to catch only EKamScriptSyntaxError,
          and adds position information to error message. }
        on E: EKamScriptFunctionArgumentsError do
          raise EKamScriptParserError.Create(Lexer, E.Message);
      end;
    except Result.Free; raise end;
  finally Lexer.Free end;
end;

end.