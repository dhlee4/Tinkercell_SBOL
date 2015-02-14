#include "SyntaxHighlighter.h"
namespace Tinkercell
{
	 /********************************
	      SYNTAX HIGHLIGHTER
	 *********************************/
	 CodingWindowSyntaxHighlighter::CodingWindowSyntaxHighlighter(QTextDocument *parent)
		 : QSyntaxHighlighter(parent)
	 {
		 HighlightingRule rule;

		 keywordFormat.setForeground(Qt::darkBlue);
		 keywordFormat.setFontWeight(QFont::Bold);
		 QStringList keywordPatterns;
		 
		 keywordPatterns << "\\bchar\\b" << "\\bconst\\b" << "\\bdouble\\b" 
						 << "\\bint\\b" << "\\blong\\b" << "\\bshort\\b" 
						 << "\\bsigned\\b" << "\\bstatic\\b" << "\\bstruct\\b"
						 << "\\btypedef\\b" << "\\btypename\\b" << "\\b#define"
						 << "\\bunsigned\\b" << "\\bvoid\\b" 
						 << "\\bOBJ\\b" << "\\bimport\\b" << "\\bfor\\b" << "\\bwhile\\b"
						 << "\\bArray\\b" << "\\barray\\b" << "\\btc_matrix\\b"
						 << "\\btc_strings\\b" << "\\btc_items\\b" << "\\btc_table\\b"
						 << "\\bmatrix\\b" << "\\bTCFunctions\\b" << "\\breturn\\b"
						 << "\\bif\\b" << "\\belse\\b" << "\\belif\\b" << "\\bdef\\b"
 						 << "\\bimport\\b" << "\\brequire\\b" << "\\bput\\b"
						 << "\\bfunction\\b";
		 foreach (QString pattern, keywordPatterns) 
		 {
			 rule.pattern = QRegExp(pattern);
			 rule.format = keywordFormat;
			 highlightingRules.append(rule);
		 }
		 
		 loopFormat.setFontWeight(QFont::Bold);
		 loopFormat.setForeground(Qt::blue);
		 rule.pattern = QRegExp("(for)|(while)|(end)");
		 rule.format = loopFormat;
		 highlightingRules.append(rule);
		 
		 classFormat.setFontWeight(QFont::Bold);
		 classFormat.setForeground(Qt::darkMagenta);
		 rule.pattern = QRegExp("\\b[A-Za-z_]+[A-Za-z0-9_]+\\.");
		 rule.format = classFormat;
		 highlightingRules.append(rule);

		 singleLineCommentFormat.setForeground(Qt::red);
		 rule.pattern = QRegExp("(//|#|%)[^\n]*");
		 rule.format = singleLineCommentFormat;
		 highlightingRules.append(rule);

		 multiLineCommentFormat.setForeground(Qt::red);

		 quotationFormat.setForeground(Qt::darkGreen);
		 rule.pattern = QRegExp("(\"[^\"]*\")|('[^']*')");
		 rule.format = quotationFormat;
		 highlightingRules.append(rule);

		 functionFormat.setFontItalic(true);
		 functionFormat.setForeground(Qt::blue);
		 rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\s*\\()");
		 rule.format = functionFormat;
		 highlightingRules.append(rule);

		 commentStartExpression = QRegExp("/\\*");
		 commentEndExpression = QRegExp("\\*/");
	 }

	 void CodingWindowSyntaxHighlighter::highlightBlock(const QString &text)
	 {
		 foreach (HighlightingRule rule, highlightingRules) {
			 QRegExp expression(rule.pattern);
			 int index = text.indexOf(expression);
			 while (index >= 0) {
				 int length = expression.matchedLength();
				 setFormat(index, length, rule.format);
				 index = text.indexOf(expression, index + length);
			 }
		 }
		 setCurrentBlockState(0);

		 int startIndex = 0;
		 if (previousBlockState() != 1)
			 startIndex = text.indexOf(commentStartExpression);

		 while (startIndex >= 0) {
			 int endIndex = text.indexOf(commentEndExpression, startIndex);
			 int commentLength;
			 if (endIndex == -1) {
				 setCurrentBlockState(1);
				 commentLength = text.length() - startIndex;
			 } else {
				 commentLength = endIndex - startIndex
								 + commentEndExpression.matchedLength();
			 }
			 setFormat(startIndex, commentLength, multiLineCommentFormat);
			 startIndex = text.indexOf(commentStartExpression,
													 startIndex + commentLength);
		 }
	 }
}
