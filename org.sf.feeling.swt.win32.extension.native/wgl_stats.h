lse {
		if (event.start == event.end) {
			accessible.textChanged(ACC.TEXT_INSERT, event.start, event.text.length());
		} else {
			accessible.textChanged(ACC.TEXT_DELETE, event.start, event.end - event.start);
			accessible.textChanged(ACC.TEXT_INSERT, event.start, event.text.length());	
		}
	}
	notifyListeners(SWT.Modify, event);
}
/**
 * Sends the specified selection event.
 */
void sendSelectionEvent() {
	getAccessible().textSelectionChanged();
	Event event = new Event();
	event.x = selection.x;
	event.y = selection.y;
	notifyListeners(SWT.Selection, event);
}
/**
 * Sets the alignment of the widget. The argument should be one of <code>SWT.LEFT</code>, 
 * <code>SWT.CENTER</code> or <code>SWT.RIGHT</code>. The alignment applies for all lines.  
 * 
 * @param alignment the new alignment
 *  
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * 
 * @see #setLineAlignment(int, int, int)
 *  
 * @since 3.2
 */
public void setAlignment(int alignment) {
	checkWidget();
	alignment &= (SWT.LEFT | SWT.RIGHT | SWT.CENTER);
	if (alignment == 0 || this.alignment == alignment) return;
	this.alignment = alignment;
	resetCache(0, content.getLineCount());
	setCaretLocation();
	super.redraw();
}
/**
 * @see org.eclipse.swt.widgets.Control#setBackground
 */
public void setBackground(Color color) {
	checkWidget();
	background = color;
	super.redraw();
}
/**
 * Sets the receiver's caret.  Set the caret's height and location.
 * 
 * </p>
 * @param caret the new caret for the receiver
 *
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setCaret(Caret caret) {
	checkWidget ();
	super.setCaret(caret);
	caretDirection = SWT.NULL;
	if (caret != null) {
		setCaretLocation();
	}
}
/**
 * Sets the BIDI coloring mode.  When true the BIDI text display
 * algorithm is applied to segments of text that are the same
 * color.
 *
 * @param mode the new coloring mode
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * 
 * @deprecated use BidiSegmentListener instead.
 */
public void setBidiColoring(boolean mode) {
	checkWidget();
	bidiColoring = mode;
}
/**
 * Moves the Caret to the current caret offset.
 */
void setCaretLocation() {
	Point newCaretPos = getPointAtOffset(caretOffset);
	setCaretLocation(newCaretPos, getCaretDirection());
}
void setCaretLocation(Point location, int direction) {
	Caret caret = getCaret();
	if (caret != null) {
		boolean isDefaultCaret = caret == defaultCaret;
		int lineHeight = renderer.getLineHeight();
		int caretHeight = lineHeight;
		if (!isFixedLineHeight() && isDefaultCaret) {
			caretHeight = getBoundsAtOffset(caretOffset).height;
			if (caretHeight != lineHeight) {
				direction = SWT.DEFAULT;
			}
		}
		int imageDirection = direction;
		if (isMirrored()) {
			if (imageDirection == SWT.LEFT) {
				imageDirection = SWT.RIGHT;
			} else if (imageDirection == SWT.RIGHT) {
				imageDirection = SWT.LEFT;
			}
		}
		if (isDefaultCaret && imageDirection == SWT.RIGHT) {
			location.x -= (caret.getSize().x - 1);
		}
		if (isDefaultCaret) {
			caret.setBounds(location.x, location.y, 0, caretHeight);
		} else {
			caret.setLocation(location);
		}
		getAccessible().textCaretMoved(getCaretOffset());
		if (direction != caretDirection) {
			caretDirection = direction;
			if (isDefaultCaret) {
				if (imageDirection == SWT.DEFAULT) {
					defaultCaret.setImage(null);
				} else if (imageDirection == SWT.LEFT) {
					defaultCaret.setImage(leftCaretBitmap);
				} else if (imageDirection == SWT.RIGHT) {
					defaultCaret.setImage(rightCaretBitmap);
				}
			}
			if (caretDirection == SWT.LEFT) {
				BidiUtil.setKeyboardLanguage(BidiUtil.KEYBOARD_NON_BIDI);
			} else if (caretDirection == SWT.RIGHT) {
				BidiUtil.setKeyboardLanguage(BidiUtil.KEYBOARD_BIDI);
			}
		}
	}
	columnX = location.x;
}
/**
 * Sets the caret offset.
 *
 * @param offset caret offset, relative to the first character in the text.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * @exception IllegalArgumentException <ul>
 *   <li>ERROR_INVALID_ARGUMENT when either the start or the end of the selection range is inside a 
 * multi byte line delimiter (and thus neither clearly in front of or after the line delimiter)
 * </ul>
 */
public void setCaretOffset(int offset) {
	checkWidget();
	int length = getCharCount();
	if (length > 0 && offset != caretOffset) {
		if (offset < 0) {
			caretOffset = 0;
		} else if (offset > length) {
			caretOffset = length;
		} else {
			if (isLineDelimiter(offset)) {
				// offset is inside a multi byte line delimiter. This is an 
				// illegal operation and an exception is thrown. Fixes 1GDKK3R
				SWT.error(SWT.ERROR_INVALID_ARGUMENT);
			}
			caretOffset = offset;
		}
		// clear the selection if the caret is moved.
		// don't notify listeners about the selection change.
		clearSelection(false);
	}
	setCaretLocation();
}	
/**
 * Copies the specified text range to the clipboard.  The text will be placed
 * in the clipboard in plain text format and RTF format.
 *
 * @param start start index of the text
 * @param length length of text to place in clipboard
 * 
 * @exception SWTError, see Clipboard.setContents
 * @see org.eclipse.swt.dnd.Clipboard#setContents
 */
void setClipboardContent(int start, int length, int clipboardType) throws SWTError {
	if (clipboardType == DND.SELECTION_CLIPBOARD && !(IS_MOTIF || IS_GTK)) return;
	TextTransfer plainTextTransfer = TextTransfer.getInstance();
	TextWriter plainTextWriter = new TextWriter(start, length);
	String plainText = getPlatformDelimitedText(plainTextWriter);
	Object[] data;
	Transfer[] types;
	if (clipboardType == DND.SELECTION_CLIPBOARD) {
		data = new Object[]{plainText};
		types = new Transfer[]{plainTextTransfer};
	} else {
		RTFTransfer rtfTransfer = RTFTransfer.getInstance();
		RTFWriter rtfWriter = new RTFWriter(start, length);
		String rtfText = getPlatformDelimitedText(rtfWriter);
		data = new Object[]{rtfText, plainText};
		types = new Transfer[]{rtfTransfer, plainTextTransfer};
	}
	clipboard.setContents(data, types, clipboardType);
}
/**
 * Sets the content implementation to use for text storage.
 *
 * @param newContent StyledTextContent implementation to use for text storage.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * @exception IllegalArgumentException <ul>
 *    <li>ERROR_NULL_ARGUMENT when listener is null</li>
 * </ul>
 */
public void setContent(StyledTextContent newContent) {
	checkWidget();	
	if (newContent == null) {
		SWT.error(SWT.ERROR_NULL_ARGUMENT);
	}
	if (content != null) {
		content.removeTextChangeListener(textChangeListener);
	}
	content = newContent;
	content.addTextChangeListener(textChangeListener);
	reset();
}
/**
 * Sets the receiver's cursor to the cursor specified by the
 * argument.  Overridden to handle the null case since the 
 * StyledText widget uses an ibeam as its default cursor.
 *
 * @see org.eclipse.swt.widgets.Control#setCursor
 */
public void setCursor (Cursor cursor) {
	if (cursor == null) {
		Display display = getDisplay();
		super.setCursor(display.getSystemCursor(SWT.CURSOR_IBEAM));
	} else {
		super.setCursor(cursor);
	}
}
/** 
 * Sets whether the widget implements double click mouse behavior.
 * </p>
 *
 * @param enable if true double clicking a word selects the word, if false
 * 	double clicks have the same effect as regular mouse clicks.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setDoubleClickEnabled(boolean enable) {
	checkWidget();
	doubleClickEnabled = enable;
}
/**
 * Sets whether the widget content can be edited.
 * </p>
 *
 * @param editable if true content can be edited, if false content can not be 
 * 	edited
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setEditable(boolean editable) {
	checkWidget();
	this.editable = editable;
}
/**
 * Sets a new font to render text with.
 * <p>
 * <b>NOTE:</b> Italic fonts are not supported unless they have no overhang
 * and the same baseline as regular fonts.
 * </p>
 *
 * @param font new font
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setFont(Font font) {
	checkWidget();
	int oldLineHeight = renderer.getLineHeight();
	super.setFont(font);	
	renderer.setFont(getFont(), tabLength);
	// keep the same top line visible. fixes 5815
	if (isFixedLineHeight()) {
		int lineHeight = renderer.getLineHeight();
		if (lineHeight != oldLineHeight) {
			int vscroll = (getVerticalScrollOffset() * lineHeight / oldLineHeight) - getVerticalScrollOffset();
			scrollVertical(vscroll, true);
		}
	}
	resetCache(0, content.getLineCount());
	claimBottomFreeSpace();	
	calculateScrollBars();
	if (isBidiCaret()) createCaretBitmaps();
	caretDirection = SWT.NULL;
	setCaretLocation();
	super.redraw();
}
/**
 * @see org.eclipse.swt.widgets.Control#setForeground
 */
public void setForeground(Color color) {
	checkWidget();
	foreground = color;
	super.setForeground(getForeground());
	super.redraw();
}
/** 
 * Sets the horizontal scroll offset relative to the start of the line.
 * Do nothing if there is no text set.
 * <p>
 * <b>NOTE:</b> The horizontal index is reset to 0 when new text is set in the 
 * widget.
 * </p>
 *
 * @param offset horizontal scroll offset relative to the start 
 * 	of the line, measured in character increments starting at 0, if 
 * 	equal to 0 the content is not scrolled, if > 0 = the content is scrolled.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setHorizontalIndex(int offset) {
	checkWidget();
	if (getCharCount() == 0) {
		return;
	}	
	if (offset < 0) {
		offset = 0;
	}
	offset *= getHorizontalIncrement();
	// allow any value if client area width is unknown or 0. 
	// offset will be checked in resize handler.
	// don't use isVisible since width is known even if widget 
	// is temporarily invisible
	if (clientAreaWidth > 0) {
		int width = renderer.getWidth();
		// prevent scrolling if the content fits in the client area.
		// align end of longest line with right border of client area
		// if offset is out of range.
		if (offset > width - clientAreaWidth) {
			offset = Math.max(0, width - clientAreaWidth);
		}
	}
	scrollHorizontal(offset - horizontalScrollOffset, true);
}
/** 
 * Sets the horizontal pixel offset relative to the start of the line.
 * Do nothing if there is no text set.
 * <p>
 * <b>NOTE:</b> The horizontal pixel offset is reset to 0 when new text 
 * is set in the widget.
 * </p>
 *
 * @param pixel horizontal pixel offset relative to the start 
 * 	of the line.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * @since 2.0
 */
public void setHorizontalPixel(int pixel) {
	checkWidget();
	if (getCharCount() == 0) {
		return;
	}	
	if (pixel < 0) {
		pixel = 0;
	}
	// allow any value if client area width is unknown or 0. 
	// offset will be checked in resize handler.
	// don't use isVisible since width is known even if widget 
	// is temporarily invisible
	if (clientAreaWidth > 0) {
		int width = renderer.getWidth();
		// prevent scrolling if the content fits in the client area.
		// align end of longest line with right border of client area
		// if offset is out of range.
		if (pixel > width - clientAreaWidth) {
			pixel = Math.max(0, width - clientAreaWidth);
		}
	}
	scrollHorizontal(pixel - horizontalScrollOffset, true);
}
/**
 * Sets the line indentation of the widget.
 * <p>
 * It is the amount of blank space, in pixels, at the beginning of each line. 
 * When a line wraps in several lines only the first one is indented. 
 * </p>
 * 
 * @param indent the new indent
 *  
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * 
 * @see #setLineIndent(int, int, int)
 *  
 * @since 3.2
 */
public void setIndent(int indent) {
	checkWidget();
	if (this.indent == indent || indent < 0) return;
	this.indent = indent;
	resetCache(0, content.getLineCount());
	setCaretLocation();
	super.redraw();	
}
/**
 * Sets whether the widget should justify lines.  
 * 
 * @param justify whether lines should be justified
 *  
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 * 
 * @see #setLineJustify(int, int, boolean)
 *  
 * @since 3.2
 */
public void setJustify(boolean justify) {
	checkWidget();
	if (this.justify == justify) return;
	this.justify = justify;
	resetCache(0, content.getLineCount());
	setCaretLocation();
	super.redraw();	
}
/** 
 * Maps a key to an action.
 * <p>
 * One action can be associated with N keys. However, each key can only 
 * have one action (key:action is N:1 relation).
 * </p>
 *
 * @param key a key code defined in SWT.java or a character. 
 * 	Optionally ORd with a state mask.  Preferred state masks are one or more of
 *  SWT.MOD1, SWT.MOD2, SWT.MOD3, since these masks account for modifier platform 
 *  differences.  However, there may be cases where using the specific state masks
 *  (i.e., SWT.CTRL, SWT.SHIFT, SWT.ALT, SWT.COMMAND) makes sense.
 * @param action one of the predefined actions defined in ST.java. 
 * 	Use SWT.NULL to remove a key binding.
 * @exception SWTException <ul>
 *    <li>ERROR_WIDGET_DISPOSED - if the receiver has been disposed</li>
 *    <li>ERROR_THREAD_INVALID_ACCESS - if not called from the thread that created the receiver</li>
 * </ul>
 */
public void setKeyBinding(int key, int action) {
	checkWidget();
	int modifierValue = key & SWT.MODIFIER_MASK;
	char keyChar = (char)(key & SWT.KEY_MASK);
	if (Compatibility.isLetter(keyChar)) {
		// make the keybinding case insensitive by adding it
		// in its upper and lower case form
		char ch = Character.toUpperCase(keyChar);
		int newKey = ch | modifierValue;
		if (action == SWT.NULL) {
			keyActionMap.remove(new Integer(newKey));
		} else {
		 	keyActionMap.put(new Integer(newKey), new Integer(action));
		}
		ch = Character.toLowerCase(keyChar);
		newKey = ch | modifierValue;
		if (action == SWT.NULL) {
			keyActionMap.remove(new Integer(newKey));
		} else {
		 	keyActionMap.put(new Integer(newKey), new Integer(action));
		}
	} else {
		if (action == SWT.NULL) {
			keyActionMap.remove(new Integer(key));
		} else {
		 	keyActionMap.put(new Integer(key), new Integer(action));
		}
	}		
}
/**
 * Sets the alignment of the specified lines. The argument should be one of <code>SWT.LEFT</code>, 
 * <code>SWT.CENTER</code> or <code>SWT.RIGHT</code>.
 * <p>
 * Should not be called if a LineStyleListener has been set since the listener 
 * maintains the line attributes.
 * </p><p>
 * All line attributes are maintained relative to the line text, not the 
 * line index that is specified in this method call.
 � During text changes, when     �e lines are inserted �y5emoved� the line 
 * attributes that a�e associated withy5he lines aft�r the change 
 * will "move" wi�h their respective text. An    �re line is defined as�y5 exten�ing from the first character on�a line to the last and incl    � the 
 * line delimiter. 
 * </�><p>
 * When two lines are join�d by deleting a line delimiter,�the top line 
 * attributes tak� precedence a�y5he attributes �f the bottom line are deleted. � * For all other text changes l�ne attributes will remain u    �ged. 
 *   
 �y5aram startLine�first line the alignment is app�ied to, 0 bas�y5* @param lineC�unt number of lines t�y5lignme�t applies to.
 * y5ar�y5li    �t line alignment
y5 
�y5except�on SWTExcepti�y5ul>
�y5      �RROR_WIDGET_DISPOSED - if t    �ceiver has be�y5isposed</li>
 �    <li>ERROR�y5EAD_INVALI    �ESS - if not called from the th�ead that created the receiv    �i>
 * </ul>
 * @exception I    �lArgumentException <u�y5*     �>ERROR_INVALID_ARGUME�y5hen th� specified li�y5ange is invali�</li>
 * </ul>
 * @see #setAlig�ment(int)
 * @since 3�y5*/
pub�ic void setLineAlignment(in    �rtLine, int lineCount, int     �ment) {
	checkWidget();
	if (is�istening(LineGetStyle�y5eturn;�	if (startLine < 0 || startLine�+ lineCount > content.getLi    �nt()) {
		SWT.error(SWT.ERR    �VALID_ARGUMENT);
	}

	renderer.�etLineAlignment(startLine, line�ount, alignment);
	resetCache(s�artLine, lineCount);
	redrawLin�s(startLine, lineCount);
	int c�retLine = getCaretLine();
	if (�tartLine <= caretLine && ca    �ne < startLine + lineCount) {
	�setCaretLocation();
	}
}
/** 
 � Sets the background color of t�e specified lines.
 * <p>
 * Th� background colory5s drawn for �he width of t�y5idge�y5ll
 * �ine background colors are disca�ded when setText is called.
 * �he text background color if def�ned in a StyleRange overlay    � 
 * line background color. 
 *�</p><p>
 * Shouldy5ot be called�if a LineBackgroundListener has�been set since the 
 * listener�maintains the line backgrou    � * </p><p>
 * All line attribut�s are maintained relative t    � line text, not the 
�y5ine in�ex that is specified �y5his me�hod call.
 * During text change�, when entire lines a�y5nserte� or removed, the line�y5 attri�utes that are associated with t�e lines after they5hange 
     �l "move" with their respective �ext. An entire line is defi    �s 
 * extending from the first �haracter on a line to the last �nd including the 
 * line delim�ter. 
 * </p><p>
 * When two li�es are joined by deleting a lin� delimiter, the top line 
 * at�ributes take preceden�y5nd the�attributes of they5ottom line a�e deleted. 
 * For all other te�t changes line attributes will �emain unchanged. y5* </p>
 * 
 � @param startLiney5ir�y5ine th� color is appliedy5o,�y5ased
 � @param lineCount number of lin�s the color applies t�y5*     �m background line background co�or
 * @exception SWTException <�l>
 *    <li>ERROR_WIDGET_DISPO�ED - if the receiver has been d�sposed</li>
 *    <li�y5OR    �AD_INVALID_ACCESS - if not call�d from the thready5hat created �he receiver</li>
y5 </ul>
     �ception IllegalArgumentExceptio� <ul>
 *   <li>ERROR_INVALID_AR�UMENT when the specified line r�nge is invalid</li>
 * </ul>
 *�
public void setLineBackground(�nt startLine, int lineCount, Co�or background) {
	checkWidg    �	
	if (isListening(LineGetBackg�ound)) return;
	if (startLi    �0 || startLine + lineCount > co�tent.getLineCount()) {
		SWT.er�or(SWT.ERROR_INVALID_ARGUMENT);�	}
	if (background != null) {
	�renderer.setLineBackground(star�Line, lineCount, background);
	� else {
		renderer.clearLin    �ground(startLine,y5ineCount    �
	redrawLines(startLine, lineCo�nt);
}
/**
 * Sets the bullet o� the specified lines.
 * <p>
 *�Should not be called �y5 LineS�yleListener h�y5een set si    �he listener 
�y5aintains t    �ne attributes�y5y5/p�y5
     � line attributes are maintained�relative to the line text, not �he 
 * line indexy5hat is speci�ied in this method call.
 *    �ng text changes, when entire li�es are insert�y5r removed, the�line 
 * attributes that ar    �ociated with the lines afte    � change 
 * will "mov�y5ith th�ir respective text. An entire l�ne is defined as 
 * extending �rom the first charact�y5n     �e to the last andy5ncluding the�
 * line delimiter. 
�y5/p><p>� * When two lines are joined by�deleting a li�y5elimiter, the �op line 
 * attributes take pre�edence and the attributes of th� bottom line are deleted. 
 * F�r all other text changes li    �tributes will remain unchanged.� * </p>
 *
 * @param startLine �irst line the bullet �y5pp    �to, 0 based
 �y5aram lineCount�number of lines the bullet appl�es to.
 * @param bull�y5ine bu�let
 * 
 * @exception SWTExcept�on <ul>
 *    <li>ERROR_WIDGET_�ISPOSED - if the receiver has b�en disposed</li>
y5    <li>ERRO�_THREAD_INVALID_ACCESS - if    �called from the thread that cre�ted the receiver</li>
 * </ul>
�* @exception IllegalArgumentExc�ption <ul>
 *   <li>ERROR_INVAL�D_ARGUMENT wh�y5he specified l�ne range is invalid</li>
 *    �>
 * @since 3�y5*/
public void�setLineBullet(int startLine, in� lineCount, Bullet bullet) {
	c�eckWidget();
	if (isListening(L�neGetStyle)) return;	
	if (star�Line < 0 || startLine�y5in    �t > content.getLineCount())    �SWT.error(SWT.ERROR_INVALID    �MENT);
	}

	renderer.setLineBul�et(startLine, lineCount, bullet�;
	resetCache(startLine, lineCo�nt);
	redrawLines(startLine, li�eCount);
	int caretLi�y5 getCa�etLine();
	if (startLine <= car�tLine && caretLine < startLine � lineCount) {
		setCaretLocatio�();
	}
}
void setVariableLineHe�ght () {
	if (!fixedLineHeight)�return;
	fixedLineHeight = fals�;
	renderer.calculateIdle()    �**
 * Sets the indent of the sp�cified lines.
 * <p>
�y5hould �ot be called �y5y5ineStyleList�ner has been set since the list�ner 
 * maintains the line attr�butes.
 * </p><p>
 * All li    �tributes are maintain�y5el    � to the line text, not the 
 * �ine index that isy5pecified    �his method call.
y5 During text�changes, when entire lines are �nserted or removed, t�y5ine 
 � attributes that are associ    �with the lines after the change�
 * will "move" with their resp�ctive text. An entire line is d�fined as 
 * extending from the�first charact�y5n a line to th� last and including t�y5 * lin� delimiter. 
 * </p><�y5* When�two lines are joined �y5eletin� a line delimiter, the top line�
 * attributes take precede    �nd the attributesy5f the bo    �line are deleted.y5 * For all o�her text changes line attribute� will remain unchange�y5* </p>� *
 * @param startLine first li�e the indent is appli�y5o, 0 b�sed
 * @param lineCou�y5um    �f lines the indent applies to.
�* @param indent line indent
 * � * @exception SWTException <ul>� *    <li>ERROR_WIDGET_DISPOSED�- if the receivery5as been disp�sed</li>
 *    <li>ERROR_THREAD�INVALID_ACCESS - if n�y5alled �rom the thread that created    �receiver</li>
 * </ul�y5 @exce�tion IllegalArgumentException <�l>
 *   <li>ERROR_INVALID_ARGUM�NT when the specified line rang� is invalid</li>
y5 </ul>
 * @s�e #setIndent(int)
 * @since 3.2� */
public vo�y5etLineIndent(i�t startLine, int lineCount, int�indent) {
	checkWidget();
	if (�sListening(LineGetStyle)) retur�;
	if (startLine < 0 �y5ta    �e + lineCount�y5ontent.get    �ount()) {
		SWT.error(SWT.ERROR�INVALID_ARGUMENT);
	}�y5endere�.setLineIndent(startLine, lineC�unt, indent);
	resetCache(s    �ine, lineCount);
	redrawLines(s�artLine, lineCount);
	int caret�ine = getCaretLine();
	if (star�Line <= caretLiney5& caretL    � startLine + lineCoun�y5
		set�aretLocation();
	}
}
/**
 * Set� the justify �y5he specified l�nes.
 * <p>
 * Should not b    �led if a LineStyleListener has �een set since the listener 
 * �aintains the line attributes.
 � </p><p>
 * All line attributes�are maintained relati�y5o the �ine text, not the 
 * line inde� that is specified in this     �d call.
 * Duringy5ext changes,�when entire linesy5re inserted �r removed, the line 
�y5ttribu�es that are associated with the�lines after t�y5hang�y5*     �"move" with their respective te�t. An entire liney5s define    �
 * extending from the first ch�racter on a line to t�y5ast an� including th�y5* li�y5elimit�r. 
 * </p><p>
 * When two     � are joined by deleti�y5 line �elimiter, the topy5in�y5*     �butes take precedence and the a�tributes of t�y5ottom line are�deleted. 
 * For all other text�changes line attribut�y5ill re�ain unchanged�y5y5/p�y5  
 * �param startLi�y5irst line     �ustify is appliedy5o,�y5ased
 � @param lineCount number of lin�s the justify applies to.
 * @p�ram justify true if lines s    � be justified
 * 
 * @exception�SWTException <ul>
 *    <li>ERR�R_WIDGET_DISPOSED - if the rece�ver has been disposed</li>
 *  � <li>ERROR_THREAD_INVALID_ACCES� - if not called from the threa� that created they5eceiver<    � * </ul>
 * @exception IllegalA�gumentExcepti�y5ul>
�y5 <    �ROR_INVALID_ARGUMENT when the s�ecified line range is invalid</�i>
 * </ul>
 * @see #setJus    �boolean)
 * @since 3.�y5/
publ�c void setLineJustify(int start�ine, int lineCount, boolean jus�ify) {
	checkWidget();
	if (isL�stening(LineGetStyle)) return;
�if (startLine < 0 || startLine � lineCount > content.getLineCou�t()) {
		SWT.error(SWT.ERRO    �ALID_ARGUMENT);
	}

	renderer.s�tLineJustify(startLin�y5ineCou�t, justify);
	resetCache(startL�ne, lineCount);
	redrawLine    �rtLine, lineCount);
	int caretL�ne = getCaretLine();
	if (s    �ine <= caretLine && caretLine <�startLine + lineCount) {
		setC�retLocation();
	}
}
/�y5* Sets�the line spacing of t�y5id    �The line spacing applies fo    � lines.
 * 
 �y5aram lineSpaci�g the line spacing
 * @exceptio� SWTException <ul>
 *�y5<li>ER�OR_WIDGET_DISPOSED - �y5he rec�iver has been disposed</li>
 * �  <li>ERROR_THREAD_INVALID_    �S - if not calledy5rom the thre�d that created the receiver</li�
 * </ul>
 * @since 3�y5*/
pub�ic void setLineSpacing(int     �pacing) {
	checkWidget();
	if (�his.lineSpaci�y5= lineSpacing �| lineSpacing < 0) return;
	thi�.lineSpacing = lineSpacing;	
	s�tVariableLineHeight()�y5esetCa�he(0, content.getLineCount());
�setCaretLocation();
	super.redr�w();
}
void setMargins (int lef�Margin, int topMargin, int     �Margin, int bottomMargin) {    �ckWidget();
	this.leftMargi    �eftMargin;
	this.topMargin = to�Margin;
	this.rightMargin = rig�tMargin;
	this.bottomMargin    �ttomMargin;
	setCaretLocati    �
}
/**
 * Fli�y5election ancho� based on word selection di    �on.
 */
void setMouseWordSelect�onAnchor() {
	if (mouseDoubleCl�ck) {
		if (caretOffs�y5 doubl�ClickSelection.x)y5
			sele    �Anchor = doubleClickSelection.y�
		} else if (caretOffset > dou�leClickSelection.y) {
			select�onAnchor = doubleClickSelection�x;
		}
	}
}
/**
 * Se�y5he    �ntation of the receiver, which �ust be one
 * of the constants �code>SWT.LEFT�y5RIGHT</code> o� <code>SWT.RIGHT_TO_LEFT</code>�
 *
 * @param orientation n    �ientation sty�y5* 
 �y5xc    �n SWTException <ul>
 �y5 <    �ROR_WIDGET_DISPOSED - if the re�eiver has been disposed</li>
 *�   <li>ERROR_THREAD_INVALID_ACC�SS - if not called fr�y5he    �ad that creat�y5he receiver</l�>
 * </ul>
 * 
 * @since 2.1.2
�*/
public void setOrientati    �t orientation) {
	if ((orientat�on & (SWT.RIGHT_TO_LE�y5 S    �FT_TO_RIGHT)) == 0) { 
		re    �
	}
	if ((orientation & SWT.RIG�T_TO_LEFT) !=�y5& (orientation�& SWT.LEFT_TO_RIGHT) != 0) {
		�eturn;	
	}
	if ((orientation & �WT.RIGHT_TO_LEFT) != 0 && isMir�ored()) {
		return;	
�y5	if ((�rientation & SWT.LEFT�y5RIGHT)�!= 0 && !isMirrored()) {
		    �n;
	}
	if (!BidiUtil.setOrienta�ion(handle, orientation)) {
		r�turn;
	}
	isMirrored = (orienta�ion & SWT.RIGHT_TO_LEFT) !=    �caretDirection = SWT.NULL;
    �tCache(0, content.getLineCount(�);
	setCaretLocation();
	keyAct�onMap.clear();
	createKeyBi    �s();
	super.redraw();
}
/**    �djusts the maximum and the page�size of the scroll ba�y5o 
 * �eflect content width/length    �ges.
 * 
 * @param vertical ind�cates if the vertical scrollbar�also needs to be set 
 */
void �etScrollBars(boolean vertic    �
	int inactive = 1;
	�y5ve    �l || !isFixedLineHeight())     �crollBar verticalBar = getVerti�alBar();
		if (verticalBar != n�ll) {
			int maximum = renderer�getHeight();
			// on�y5et the�real values if the scroll b    �n be used 
			// (ie. because t�e thumb size is less than t    �roll maximum)
			// avoids     �ing on Motif, fixes 1G7RE1J and�1G5SE92
			if (clientAreaHeight�< maximum) {
				verticalBar.se�Values(
					verticalBar.getSel�ction(),
					verticalBar.getMi�imum(),
					maximum,
					clie�tAreaHeight,				// thumb si    �			verticalBar.getIncrement(),
�				clientAreaHeight);				// pa�e size
			} else if (vertic    �.getThumb() != inacti�y5| vert�calBar.getMaximum() != inactive� {
				verticalBar.setValue    �			verticalBar.getSelection(),
�				verticalBar.getMinimum(),
	�			inactive,
					inactive,
			�	verticalBar.getIncrement(),
		�		inactive);
			}
		}
	}
	Scrol�Bar horizontalBar = getHorizont�lBar();
	if (horizontalBar != n�ll && horizontalBar.getVisi    �) {
		int maximum = renderer.ge�Width();
		// only set the real�values if the scroll bar ca    �used 
		// (i�y5ecau�y5he thu�b size is less than t�y5croll �aximum)
		// avoids flashing on�Motif, fixes 1G7RE1J and 1G5SE9�
		if (clientAreaWidth < maximu�) {
			horizontalBar.setValues(�				horizontalBar.getSelection(�,
				horizontalBar.getMini    �,
				maximum,
				clientAreaWi�th - leftMarg�y5y5ightMargin,	�/ thumb size
				horizontal    �etIncrement(),
				clientAr    �th - leftMarg�y5y5ightMargin);�// page size
		} else if (h    �ntalBar.getThumb() != inact    �| horizontalBar.getMaximum() !=�inactive) {
			horizontalBa    �Values(
				horizontalBar.getSe�ection(),
				horizontalBar.get�inimum(),
				inactive,
				ina�tive,
				horizontalBar.getIncr�ment(),
				inactive);
		}
	}
}�/** 
 * Sets the selection     �e given position and scroll    �into view.  Equivalent to setSe�ection(start,start).
�y5*     �m start new carety5osition
    �ee #setSelection(int,int)
 * @e�ception SWTException <ul>
     �<li>ERROR_WIDGET_DISPOSED -    �he receiver h�y5een dispos    �i>
 *    <li>ERROR_THREAD_INVAL�D_ACCESS - if noty5alled from t�e thread that created the recei�er</li>
 * </ul>
y5 @exception �llegalArgumentExcepti�y5ul>
 *�  <li>ERROR_INVALID_ARGUMEN    �n either the start or the end o� the selection range �y5ns    � 
 * multi by�y5ine delimiter �and thus neither clearly in fro�t of or after they5ine deli    �)
 * </ul> 
 �y5ublic void set�election(int start) {
	// check�idget test do�y5n setSelection�ange	
	setSelection(start,     �);
}
/** 
 * Setsy5he selection�and scrolls it into view.
     �
 * Indexing is zero based.  Te�t selections are specified in t�rms of
 * car�y5ositions.  In � text widget thaty5ontains N ch�racters, there are 
 * N+1 care� positions, ranging from 0..N
 � </p>
 *
 * @param point x=sele�tion start offset, y=selection �nd offset
 * 	They5ar�y5ill be�placed at the selecti�y5ta    �en x > y.
 * @seey5setSelection�int,int)
 * @exception SWTExcep�ion <ul>
 *    <li>ERROR_WIDGET�DISPOSED - if they5eceiver has �een disposed</li>y5* �y5li>ERR�R_THREAD_INVALID_ACCE�y5 if no� called from the thread that cr�ated the receiver</li�y5 </ul>� * @exception IllegalArgumentEx�eption <ul>
 *   <li>ERROR_    �ARGUMENT when point is null</li�
 *   <li>ERROR_INVALID_ARGUMEN� when either the start or the e�d of the selection range is ins�de a 
 * multi byte line de    �er (and thus neither clearl    �front of or aftery5he line     �iter)
 * </ul> 
 */
public void�setSelection(Point point) {
	ch�ckWidget();
	if (point == n    �SWT.error (SWT.ERROR_NULL_ARGUM�NT);	
	setSelection(point.x    �nt.y);
}
/**
 * Sets the receiv�r's selection background color �o the color specified
 * by    �argument, or to the default sys�em color for the control
 * if �he argument is null.
�y5*     �m color the n�y5olor (or null)� *
 * @exception IllegalArgumen�Exception <ul>
 *    <li>ER    �NVALID_ARGUME�y5y5f the ar    �t has been disposed</li> 
 * </�l>
 * @exception SWTException <�l>
 *    <li>ERROR_WIDGET_DISPO�ED - if the receiver has been d�sposed</li>
 *    <li�y5OR    �AD_INVALID_ACCESS - if not call�d from the thready5hat created �he receiver</li>
y5 </ul>
     �nce 2.1
 */
public void setSele�tionBackground (Color color) {
�checkWidget ();
	if (color     �ll) {
		if (color.isDisposed())�SWT.error(SWT.ERROR_INVALID    �MENT);
	}
	selectionBackground � color;
	super.redraw();
}
    �* Sets the receiver's selection�foreground color to t�y5olor s�ecified
 * by they5rgument, or �o the default system color     �he control
 * if the argument i� null.
 *
 * @param color the n�w color (or null)y5*
�y5ex    �on IllegalArgumentException <ul�
 *    <li>ERROR_INVALID_AR    �T - if the argument h�y5een di�posed</li> 
 * </ul>
�y5except�on SWTExcepti�y5ul>
�y5      �RROR_WIDGET_DISPOSED - if t    �ceiver has be�y5isposed</li>
 �    <li>ERROR�y5EAD_INVALI    �ESS - if not called from the th�ead that created the receiv    �i>
 * </ul>
 * @since 2.1
     �blic void setSelectionForeg    � (Color color) {
	checkWidget (�;
	if (color != null) {
		if (c�lor.isDisposed()) SWT.error(SWT�ERROR_INVALID_ARGUMENT);
	}
	se�ectionForeground = color;
	supe�.redraw();
}
/** 
 * Sets the s�lection and scrolls it into vie�.
 * <p>
 * Indexing �y5ero ba�ed.  Text selections are sp    �ed in terms o�y5y5ar�y5os    �s.  In a text widget that conta�ns N characters, there are 
 * �+1 caret positions, ranging fro� 0..N
 * </p>
 *
 * @param star� selection start offset. The ca�et will be placedy5t the 
 * 	s�lection start when start > end.� * @param end selecti�y5nd off�et
 * @see #setSelectionRange(i�t,int)
 * @exception SWTExcepti�n <ul>
 *    <li>ERROR_WIDGET_D�SPOSED - if t�y5eceiver has be�n disposed</li>
 *    <li>E    �THREAD_INVALID_ACCESS�y5f not �alled from the thread that crea�ed the receiver</li>
�y5/ul>
 � @exception IllegalArgumentExce�tion <ul>
 *   <li>ERROR_INVALI�_ARGUMENT when either the s    �or the end of they5election ran�e is inside a�y5y5ul�y5yte li�e delimiter (and thus neith    �early in front ofy5r after     �ine delimiter�y5y5/u�y5*/    �ic void setSelection(int st    �int end) {
	setSelectionRange(s�art, end - start);
	showSelecti�n();
}
/** 
 * Sets t�y5electi�n.
 * <p>
 * The new selection �ay not be visible. Ca�y5ho    �ction to scro�y5y5 t�y5electi�n into view.
�y5/p>
�y5*     �m start offset ofy5he first sel�cted characte�y5tart >= 0 must�be true.
 * @param length numbe� of characters toy5elect, 0    �tart + length�y5y5<= getCharCo�nt() must be true. 
 * 	A negat�ve length places the caret     �e selection start.
 * @param se�dEvent a Selection event is sen� when set to truey5nd when     �the selection is rese�y5*/
voi� setSelection(int start, int le�gth, boolean sendEven�y5
	int �nd = start + length;
	if (start�> end) {
		int temp = end;
		en� = start;
		start = temp;
	}
	/� is the selection ran�y5iffere�t or is the selection direction�
	// different?
	if (select    � != start || selection.y !=    �|| 
		(length�y5y5& selectionA�chor != selection.x) �y5		(len�th < 0 && selectionAnchor !    �ection.y)) {
		clearSelection(s�ndEvent);
		if (length < 0) {
	�	selectionAnchor = selection.y � end;
			caretOffset = selectio�.x = start;
		} else {
			s    �ionAnchor = selection�y5 s    �
			caretOffs�y5y5election.y =�end;
		}
		internalRedrawRange(�election.x, selection�y5 selec�ion.x);
	}
}
/** 
 * Sets the s�lection.
 * <p>
 * The new sele�tion may not be visible. Call s�owSelection to scroll the selec�ion
 * into view. A negative le�gth places the caret �y5he vis�al start of t�y5election.
 * <�p>
 *
 * @par�y5tart offse    �the first selected characte    �@param length number �y5haract�rs to select
�y5y5 @exception �WTException <ul>
 *    <li>ERRO�_WIDGET_DISPOSED - if the r    �er has been disposed</li>
 *   �<li>ERROR_THREAD_INVALID_AC    �- if not call�y5rom the thread�that created the receiver</li>
�* </ul>
 * @exception IllegalAr�umentException <ul>
 �y5<l    �OR_INVALID_ARGUMENT when either�the start or the end of the sel�ction range is inside�y5 * mul�i byte line delimiter (and thus�neither clear�y5n front of or �fter the line delimiter)
 * </u�>
 */
public void setSelectionR�nge(int start, int length) {
	c�eckWidget();
	int contentLength�= getCharCount();
	start = Math�max(0, Math.min (star�y5ontent�ength));
	int end = start + len�th;
	if (end < 0) {
		length = �start;
	} else {
		if (end > co�tentLength) length = contentLen�th - start;
	}
	if (isLineD    �ter(start) || isLineDelimiter(s�art + length)) {
		// the s    �offset or end offset �y5he sel�ction range is inside�y5		// m�lti byte line delimiter. This i� an illegal operation and an ex�eption 
		// is thrown. Fixes 1�DKK3R
		SWT.error(SWT.ERROR    �LID_ARGUMENT);
	}
	setSelection�start, length, false);
	setCare�Location();
}
/** 
 * Adds     �pecified styl�y5* <p�y5 T    �w style overwrites existing sty�es for the specified range.
 * �xisting style ranges are ad    �d if they partially overlap    � 
 * the new style. To clear an�individual style,y5all setStyle�ange 
 * with a StyleRange that�has null attributes. 
 * </    �
 * Should not bey5alled if a L�neStyleListen�y5as been se    �ce the 
 * listener maintains t�e styles.
 * </p>y5*
�y5param �ange StyleRan�y5bject cont    �g the style information.
 * Ove�writes the old style �y5he giv�n range. May �y5ull �y5elete
�* all styles.
 * y5xception SWT�xception <ul>
 * y5 <li>ERR    �DGET_DISPOSED�y5f the rece    �has been disposed</li�y5    <l�>ERROR_THREAD�y5ALID�y5ESS - �f not called fromy5he thread th�t created the receiver</li>    �/ul>
 * @exception IllegalA    �ntException <ul>
 *   <li>ERROR�INVALID_RANGE when the style ra�ge is outside the val�y5ange (� getCharCount())</li>�y5 <    � */
public vo�y5etStyleRange(S�yleRange rang�y5
	checkWidget(�;
	if (isListening(LineGetStyle�) return;
	if (range �y5ull) {�		if (range.isUnstyled()) {
			�etStyleRanges(range.start, rang�.length, null, null, false)    � else {
			setStyleRanges(range�start, 0, null, new StyleRange[�{range}, false);
		}
�y5lse {
�	setStyleRanges(0, 0, null, nul�, true);
	}
}
/** 
 * Clear    � styles in the range specified �y <code>start</code> and 
 * <c�de>length</code> and adds the n�w styles.
 * <p>
y5 T�y5anges �rray contains start a�y5ength �airs.  Each pair refe�y5o
 * t�e corresponding style in the st�les array.  F�y5xample, the pa�r
 * that starts at ranges[    �th length ranges[n+1] uses     �tyle
 * at styles[n/2�y5The ra�ge fields within each Style    � are ignored.
 * If ranges or s�yles is null, the specified ran�e is cleared.
 * </p><p>
 * Not�: It is expected that the s    �nstance of a StyleRan�y5ill oc�ur
 * multiple times within the�styles array, reducing memory u�age.
 * </p><�y5* Should n    � called if a LineStyleListener �as been set sincey5he�y5 l    �er maintains the styles.
 * </p�
 *
 * @param start offset     �rst character where styles     �be deleted
 * @param length len�th of the ran�y5o delete s    � in
 * @param ranges the ar    �f ranges.  The ranges must not �verlap and mu�y5e in order.
 *�@param styles the arr�y5f Styl�Ranges.  The range fields withi� the StyleRan�y5re unused.    � * @exception SWTException <ul>� *    <li>ERROR_WIDGET_DISPOSED�- if the receivery5as been disp�sed</li>
 *    <li>ERROR_THREAD�INVALID_ACCESS - if n�y5alled �rom the thread that created    �receiver</li>
 * </ul�y5 @exce�tion IllegalArgumentException <�l>
 *    <li>ERROR_NULL_ARGUMEN� when an element in t�y5tyles �rray is null</li>
 * �y5li>ERR�R_INVALID_RAN�y5hen the nu    �of ranges and style do not matc� (ranges.leng�y5y5 == styles.l�ngth)</li> 
 �y5y5li�y5OR_INV�LID_RANGE when a range is o    �e the valid rangey5> getCharCou�t() or less than zero)</li>    �   <li>ERROR_INVALID_RANGE when�a range overlaps</li>�y5 </ul>� * 
 * @since 3.2 
 */
public v�id setStyleRanges(int start, in� length, int[] ranges, StyleRan�e[] styles) {
	checkWidget();
	�f (isListening(LineGetStyle)) r�turn;
	if (ranges == null || st�les == null) {
		setStyleRanges�start, length, null, null, fals�);
	} else {
		setStyleRanges(s�art, length, ranges, styles, fa�se);
	}
}
/** 
 * Sets styles t� be used for rendering the widg�t content.
 * <p>
 * All styles�in the widget will be replaced �ith the given sety5f ranges    �styles.
 * The ranges array con�ains start and length pairs.  E�ch pair refers toy5* the corres�onding style in the styles arra�.  For example, the pair
 * tha� starts at ranges[n] with lengt� ranges[n+1] usesy5he style    �t styles[n/2].  The range field� within each StyleRan�y5re ign�red.
 * If eithery5rgument is n�ll, the styles are cleared.    �/p><p>
 * Note: It is expected �hat the same instance of a Styl�Range will occur
y5 multiple ti�es within the styles array,    �cing memory usage.
 * </p><    � Should not be called if a     �tyleListener has been set since�the 
 * listener maintains     �tyles.
 * </p>
 *
 * @param    �es the array �y5ange�y5The ra�ges must not overlap and must b� in order.
 * @param styles the�array of StyleRanges.  The     � fields within the StyleRan    �e unused.
 * 
 * y5xception SWT�xception <ul>
 * y5 <li>ERR    �DGET_DISPOSED�y5f the rece    �has been disposed</li�y5    <l�>ERROR_THREAD�y5ALID�y5ESS - �f not called fromy5he thread th�t created the receiver</li>    �/ul>
 * @exception IllegalA    �ntException <ul>
 *    <li>ERRO�_NULL_ARGUMENT when an element �n the styles array is null</li>� *    <li>ERROR_INVALID_RANGE w�en the number of rang�y5nd sty�e do not match (ranges.length *�2 == styles.length)</li> 
     �<li>ERROR_INVALID_RAN�y5hen a �ange is outsi�y5he valid r    �(> getCharCount()y5r less than �ero)</li> 
 *    <li>ERROR_INVA�ID_RANGE when a range overlaps<�li> 
 * </ul>
 * y5* @since    �
 */
public void setStyleRanges�int[] ranges, StyleRange[] styl�s) {
	checkWidget();
	if (isLis�ening(LineGetStyle)) return;
	i� (ranges == null || styles     �ll) {
		setStyleRanges(0, 0    �l, null, true);
	} el�y5
	    �tyleRanges(0, 0, ranges, styles� true);
	}
}
void setStyleRange�(int start, int lengt�y5nt[] r�nges, StyleRange[] styles, bool�an reset) {
	int charCount     �tent.getCharCount();
	int end =�start + length;
	if (start > en� || start < 0) {
		SWT.erro    �.ERROR_INVALID_RANGE);
	}
	if (�tyles != null) {
		if (end     �rCount) {
			SWT.error(SWT.ERRO�_INVALID_RANGE);
		}
		if (rang�s != null) {
			if (ranges.leng�h != styles.length << 1) SW    �or(SWT.ERROR_INVALID_ARGUMENT);�		}
		int lastOffset = 0;
		boo�ean variableHeight = false; 
		�or (int i = 0; i < styles.l    �; i ++) {
			if (styles[i] == n�ll) SWT.error(SWT.ERROR_INV    �ARGUMENT);
			int rangeStart, r�ngeLength;
			if (ranges != nul�) {
				rangeStart = ranges[i <� 1];
				rangeLength = ranges[(� << 1) + 1];
			} else {
		    �geStart = styles[i].start;
    �angeLength = styles[i].length;
�		}
			if (rangeLength < 0) SWT�error(SWT.ERROR_INVALID_ARGUMEN�); 
			if (!(0 <= rangeStart &&�rangeStart + rangeLength <= cha�Count)) SWT.error(SWT.ERROR    �LID_ARGUMENT);
			if (lastOffse� > rangeStart) SWT.error(SWT.ER�OR_INVALID_ARGUMENT);
			variab�eHeight |= styles[i].isVariable�eight();
			lastOffset = rangeS�art + rangeLength;
		}
		if (va�iableHeight) setVariableLin    �ht();
	}
	int rangeStart = star�, rangeEnd = end;
	if (styles !� null && styles.length > 0)    �if (ranges != null) {
			rangeS�art = ranges[0];
			rangeEnd = �anges[ranges.length - 2] + rang�s[ranges.leng�y5y5];
		} else �
			rangeStart = styles[0].    �;
			rangeEnd�y5tyles[styles.l�ngth - 1].sta�y5y5tyles[st    �length - 1].length;
		}
	}
    �lastLineBottom = 0;
	�y5!i    �dLineHeight() && !reset) {
    � lineEnd = content.getLineAtOff�et(Math.max(end, rangeEnd))    �nt partialTopIndex = getPar    �opIndex();
		int partialBottomI�dex = getPartialBottomIndex();
�	if (partialTopIndex �y5ineEnd�&& lineEnd <= partialBottomInde�) {
			lastLineBottom�y5et    �ixel(lineEnd + 1);
		}
	}
	if (�eset) {
		renderer.setStyle    �s(null, null);
	} else {
		rend�rer.updateRanges(star�y5en    �length);
	}
	if (styl�y5=     �&& styles.length > 0) {
		rende�er.setStyleRanges(ranges, style�);
	}
	if (reset) {
		resetCach�(0, content.getLineCount())    �uper.redraw();
	}y5lse {
		    �ineStart = content.getLineAtOff�et(Math.min(start, rangeStart))�
		int lineEnd = content.getLin�AtOffset(Math.max(end, rangeEnd�);
		resetCache(lineStart, line�nd - lineStart + 1);
		int     �alTopIndex = getPartialTopIndex�);
		int partialBottomIndex = g�tPartialBottomIndex();
		if    �ineStart > partialBottomIndex |� lineEnd < partialTopIndex)) {
�		int y = 0;
			int height     �entAreaHeight;
			if (partialTo�Index <= lineStart && lineStart�<= partialBottomIndex) {
				in� lineTop = Math.max(y, getLineP�xel(lineStart));
				y = lineTo�;
				height -= lineTop;
			}
	�	if (partialTopIndex �y5ineEnd�&& lineEnd <= partialBottomInde�) {
				int newLastLineBott    �getLinePixel(lineEnd + 1);
				�f (!isFixedLineHeight()) {
				�scrollText(lastLineBottom, newL�stLineBottom);
				}
				height�= newLastLineBottom - y;
			}
	�	super.redraw(0, y, clientA    �dth, height, false);		
		}
    �etCaretLocation();
}
/** 
 * Se�s styles to be used for renderi�g the widget content. All style� 
 * in the widget wi�y5e repl�ced with the given set of style�.
 * <p>
 * Note: Because a Sty�eRange includes the start and l�ngth, the
 * samey5nstance cann�t occur multiple times in the a�ray of styles.
 * If the same s�yle attribute�y5uch �y5on    � color, occur in
y5 multiple St�leRanges, <code>setStyleRan    �nt[], StyleRange[])</code>
 * c�n be used to share styles a    �duce memory usage.
 * </p><    � Should not be called if a     �tyleListener has been set since�the 
 * listener maintains     �tyles.
 * </p>
 *
 * @param    �es StyleRange objects containin� the style informatio�y5* The �anges should not overlap. The s�yle rendering is undefined if 
�* the ranges �y5verlap. Must n�t be null. The styles need to b� in order.
 * @exception SWTExc�ption <ul>
 *    <li>ERROR_WIDG�T_DISPOSED - �y5he receiver ha� been disposed</li>
 *    <li>E�ROR_THREAD_INVALID_ACCESS - if �ot called from the thread that �reated the receiver</li>
 * </u�>
 * @excepti�y5llegalArgu    �xception <ul>
 * y5 <li>ERR    �LL_ARGUMENT when the list o    �ges is null</li>
 *    <li>ERRO�_INVALID_RANGE when t�y5ast of�the style ranges is outside    �valid range (�y5tCharCount())<�li> 
 * </ul>
 * y5* @see #    �yleRanges(int[], StyleRange[])
�*/
public void setStyleRang    �yleRange[] ranges) {
	checkWidg�t();
	if (isListening(LineGetSt�le)) return;
 	if (ranges == nu�l) SWT.error(SWT.ERROR_NULL_ARG�MENT);
	setStyleRanges(0, 0    �l, ranges, true);
}
/�y5 * Set� the tab width. 
 *
 �y5aram t�bs tab width measured in ch    �ers.
 * @exception SWTExcep    �<ul>
 *    <li>ERROR_WIDGET_DIS�OSED - if the receiver has     �disposed</li>
 *    <li>ERROR_T�READ_INVALID_ACCESS - if not ca�led from the thread that create� the receiver</li>
 * </ul>    �public void setTabs(i�y5abs) {�	checkWidget(�y5y5abLength = t�bs;
	renderer.setFont(null,    �);
	if (caretOffset > 0) {
    �etOffset = 0;
		showCaret();
		�learSelection(false);
	}
	reset�ache(0, content.getLineCount())�
	super.redraw();
}
/�y5 * Set� the widget content. 
 * If    �widget has the SWT.SINGLE style�and "text" contains more than 
�* one line, only the first line�is rendered b�y5he text is sto�ed 
 * unchanged. A subsequent �all to getText will return     �ame text 
 * thaty5as set.
 * <�>
 * <b>Note:</b>y5nly a si    �line of text should be set     �the SWT.SINGLE 
 * style is use�.
 * </p>
 *
�y5param text new�widget conten�y5eplaces ex    �g content. Li�y5tyle�y5* 	tha� were set usi�y5tyledText     �re discarded.  The
 * 	current �election is also discarded.    �exception SWTException <ul>    �  <li>ERROR_WIDGET_DISPOSED    � the receiver hasy5een disp    �/li>
 *    <li>ERROR_THREAD_INV�LID_ACCESS - �y5ot called from�the thread th�y5reat�y5he rec�iver</li>
 * </ul>
 * @exceptio� IllegalArgumentException <ul>
�*    <li>ERROR_NULL_ARGUMEN    �n string is null</li>
 * </    �*/
public void setText(Stri    �xt) {
	checkWidget();
	if (text�== null) {
		SWT.error(SWT.ERRO�_NULL_ARGUMENT);
	}
	Event even� = new Event();
	event.start = �;
	event.end = getCharCount();
�event.text = text;
	event.doit � true;	
	notifyListeners(SWT.Ve�ify, event);
	if (event.doit) {�		StyledTextEvent styledTex    �t = null;
		if (isListening(Ext�ndedModify)) {
			styledTextEve�t = new StyledTextEvent(con    �;
			styledTextEvent.start = ev�nt.start;
			styledTextEvent.en� = event.start + event.text    �th();
			styledTextEvent.te    �content.getTextRange(event.    �, event.end - event.start);
		}�		content.setText(event.tex    �	sendModifyEvent(event);	
	    �styledTextEve�y5= null) {
			n�tifyListeners(ExtendedModify, s�yledTextEvent);
		}
	}
}
/**
 *�Sets the text limit to the spec�fied number of characters.
    �>
 * The text limit specifies t�e amount of text that
 * the us�r can type in�y5he widget.
 * �/p>
 *
 * @param limit the new �ext limit.
 * @exception SWTExc�ption <ul>
 *    <li>ERROR_WIDG�T_DISPOSED - �y5he receiver ha� been disposed</li>
 *    <li>E�ROR_THREAD_INVALID_ACCESS - if �ot called from the thread that �reated the receiver</li>
 * </u�>
 * @excepti�y5llegalArgu    �xception <ul>
 * y5<li>ERRO    �NOT_BE_ZERO when limit is 0</li�
 * </ul>
 */
public void setTe�tLimit(int limit)y5
	checkWidge�();
	if (limit == 0) {
		SWT.er�or(SWT.ERROR_CANNOT_BE_ZERO);
	�
	textLimit = limit;
}
/**
    �ts the top index.y5o nothing if�there is no text set.
 * <p>
 *�The top index is the index of t�e line that is currently at    �top 
 * of the widget. The top �ndex changes wheny5he widget is�scrolled.
 * Indexing starts fr�m zero.
 * Note: The top index �s reset to 0 wheny5ew text is s�t in the widget.
 * </p>
 *
 * �param topIndex new top index. M�st be between�y5nd 
�y5getLin�Count() - ful�y5isib�y5ines p�r page. If no lines a�y5ully 
�* 	visible the maximum value is�getLineCount() - 1. An out of r�nge 
 * 	index will be adjusted�accordingly.
 * @exception     �ception <ul>
 *    <li>ERROR_WI�GET_DISPOSED - ify5he receiver �as been disposed</li>
 *       �ERROR_THREAD_INVALID_ACCESS - i� not called from the thread tha� created the receiver</li>
 * <�ul>
 */
public void setTopIndex�int topIndex) {
	checkWidget();�	if (getCharCount() == 0) {
		r�turn;
	}
	int lineCou�y5 conte�t.getLineCount(), pixel;
	if (i�FixedLineHeight()) {
		int page�ize = Math.max(1, Math.min(line�ount, getLineCountWhole()));
		�f (topIndex < 0) {
			topIn    � 0;
		} else �y5topIndex >    �Count - pageSize)y5
			topIndex�= lineCount - pageSize;
		}    �xel = getLinePixel(topIndex);
	� else {
		topIndex = Math.m    � Math.min(lineCount - 1, topInd�x));
		pixel = getLinePixel(top�ndex);
		if (pixel > �y5
			pi�el = getAvailableHeightBellow(p�xel);
		} else {
			pixel = get�vailableHeightAbove(pixel);
		}�	} 
	scrollVertical(pixel, true�;
}
/**
 * Se�y5he t�y5ixel o�fset. Do nothing if there is no�text set.
 * <p>
y5 T�y5op pix�l offset is t�y5ertical pi    �ffset of the widget. The
 * wid�et is scrolled so that the give� pixel position is at the top.
�* The top index is adjusted to �he corresponding top line.
 * N�te: The top pixely5s reset     �when new text is set �y5he wid�et.
 * </p>
 *
 * @param pi    �ew top pixel offset. Must be be�ween 0 and 
 �y5getLineCount()�- visible lines per page) / get�ineHeight()). An out
�y5of ran�e offset will be adjusted accor�ingly.
 * @exception SWTExcepti�n <ul>
 *    <li>ERROR_WIDGET_D�SPOSED - if t�y5eceiver has be�n disposed</li>
 *    <li>E    �THREAD_INVALID_ACCESS�y5f not �alled from the thread that crea�ed the receiver</li>
�y5/ul>
 � @since 2.0
 �y5ublic void    �opPixel(int pixel) {
	check    �t();
	if (getCharCount() == 0) �
		return;
	}	
	if (pixel < 0) �ixel = 0;
	int lineCount = cont�nt.getLineCount();
	i�y5eight � clientAreaHeighty5 topMargin -�bottomMargin;
	int verticalOffs�t = getVerticalScrollOffset    �if (isFixedLineHeight()) {
		in� maxTopPixel = Math.max(0, line�ount * getVerticalIncrement    �height);
		if (pixel �y5xTopPi�el) pixel = maxTopPixel;
		    � -= verticalOffset; 
�y5lse {
�	pixel -= verticalOffset;
	    �pixel > 0) {
�y5ixel�y5etAvai�ableHeightBellow(pixel);
		}
	}�	scrollVertical(pixel, true    �/**
 * Sets whether t�y5id    �raps lines.
 * <p>
 * This over�ides the creationy5ty�y5it    �WRAP.
 * </p>
 *
 * @param wrap�true=widget wrapsy5ines, fa    �idget does not wrap lines
 * @s�nce 2.0
 */
public void setWord�rap(boolean wrap) {
	checkWidge�();
	if ((getStyle() & SWT.SING�E) != 0) return;
	if (wordWrap �= wrap) return;
	wordWrap =    �;
	setVariableLineHeight();
	re�etCache(0, content.getLineCount�));
	horizontalScrollOffset = 0�
	ScrollBar horizontalBar =    �orizontalBar();
	if (horizontal�ar != null) {
		horizontalBar.s�tVisible(!wordWrap);
	}
	se    �llBars(true);
	setCaretLocation�);
	super.redraw();
}
/**
     �olls the specified location    � view.
 * 
 * @param x the x co�rdinate that should be made vis�ble.
 * @param line t�y5ine th�t should be made visible. Relat�ve to the
 *	first li�y5n the �ocument.
 * @return 
�y5rue=th� widget was scrolled �y5ak    � specified location visible. 
 �	false=the specified location i� already visible, the widget wa� 
 *	not scrolled. 	
 */
boolea� showLocation(Rectang�y5ect) {�	int clientAreaWidth = this    �ntAreaWidth - leftMargin - righ�Margin;
	int clientAreaHeight =�this.clientAreaHeight�y5op    �n - bottomMargin;
	boolean scro�led = false;
	if (rect.y <= top�argin) {
		scrolled = scrollVer�ical(rect.y - topMargin, tr    �	} else if (rect.y + rect.heigh� > clientAreaHeight) {
		sc    �d = scrollVertical(rect.y +    �.height - clientAreaHeight, tru�);
	}
	if (clientAreaWidth > 0)�{
		// always make 1/4 of a pag� visible
		if (rect.x�y5eftMar�in) {
			int scrollWidth = Math�max(leftMargin - rect.x, cl    �reaWidth / 4);
			int maxScroll�= horizontalScrollOffset;
			sc�olled = scrollHorizontal(-M    �in(maxScroll, scrollWidth), tru�);
		} else if (rect.x + rect.w�dth > clientAreaWidth) {
			int�scrollWidth =  Math.max(rect.x � rect.width - clientAreaWidth, �lientAreaWidth / 4);
			int max�croll = renderer.getWidth() - h�rizontalScrollOffset - this    �ntAreaWidth;
			scrolled = scro�lHorizontal(Math.min(maxScroll,�scrollWidth), true);
		}
	}
	re�urn scrolled;
}
/**
 * Sets the�caret location and scrolls     �aret offset into view�y5/
    �showCaret() {
	Rectangle bounds�= getBoundsAtOffset(caretOf    �;
	if (!showLocation(bounds)) {�		setCaretLocation();
	}
}
/**
�* Scrolls the selecti�y5nt    �w.
 * <p>
 * The end �y5he sel�ction will be scrolled into vie�.
 * Note that ify5 right-t    �t selection exists, t�y5nd of �he selection is
 * the visual b�ginning of the selection (i.e.,�where the car�y5s located).
 *�</p>
 *
 * @exception SWTExcept�on <ul>
 *    <li>ERROR_WIDGET_�ISPOSED - if the receiver has b�en disposed</li>
y5    <li>ERRO�_THREAD_INVALID_ACCESS - if    �called from the thread that cre�ted the receiver</li>
 * </ul>
�*/
public void showSelectio    �
	checkWidget();
	// �y5electi�n from right-to-left?
	boolean �ightToLeft = caretOffset == sel�ction.x;
	int startOffset, endO�fset;
	if (rightToLef�y5
		sta�tOffset = selection.y;
		en    �et = selection.x;
	} else {    �artOffset = selection.x;
		    �fset = selection.y;
	�y5	Recta�gle startBounds = getBoundsAtOf�set(startOffset);
	Rectangle en�Bounds = getBoundsAtOffset(endO�fset);
	
	// can the selection �e fully displayedy5ithin th    �get's visible width?
	int w = c�ientAreaWidth�y5oole�y5electi�nFits = rightToLeft ? startBoun�s.x - endBounds.x <= w : endBou�ds.x - startBounds.x �y5;
    �selectionFits) {
		// show as m�ch of the selection as poss    �by first showing
		// the start�of the selection
		if (showLoca�ion(startBounds)) {
			// endX �alue could changey5f showing st�rtX caused a scroll to occur
		�endBounds = getBoundsAtOffs    �dOffset);
		}
		showLocation(en�Bounds);
	} else {
		�y5us    �w the end of the selection sinc� the selection start 
		//     �not be visible
		showLocation(e�dBounds);
	}
}
/**
 * Updates t�e selection and caret position �epending on the text change.
 *�<p>
 * If the selecti�y5nt    �ts with the replaced text, the �election is 
�y5eset and t    �ret moved to the end �y5he    �text.
 * If t�y5election is be�ind the replaced text it is    �d so that the
 * same text     �ns selected.  If the selection �s before the replaced text 
 * �t is left unchanged.
�y5/p    � * @param startOffset offse    �the text change
 * @param repla�edLength leng�y5f te�y5eing r�placed
 * @param newLength leng�h of new text
 */y5oid updateSe�ection(int startOffse�y5nt rep�acedLength, int newLength) {
	i� (selection.y <= startOffset) {�		// selection ends before     �change
		return;
	}
	�y5select�on.x < startOffset) {
		//     � selection fragment before text�change
		internalRedrawRange(se�ection.x, startOffset�y5el    �n.x);
	}
	if (selection.y > sta�tOffset + replacedLength &&    �ction.x < startOffset�y5ep    �Length) {
		// clear selection �ragment after text change.
    �do this only when the selection�is actually affected �y5he    �/ change. Selection is only    �cted if it intersects the chang� (1GDY217).
		int netNewLength � newLength - replacedLength;
		�nt redrawStart = startOffse    �ewLength;
		internalRedrawRange�redrawStart, selection.y +     �wLength - redrawStart);
	}
    �selection.y > startOffset &    �ection.x < startOffset + replac�dLength) {
		// selection inter�ects replaced text. s�y5aret b�hind text change
		setSelection�startOffset + newLength, 0,    �);
	} else {
		//y5ove selectio� to keep same text selected
		s�tSelection(selection.x + newLen�th - replacedLength, selection.� - selection.x, true);
	}
	setCaretLocation();
}
}
                                                                                                                                                                                                                                                                                                                                                                                                             s, int cGlyphs, int pwLogClust, int psva, int piAdvance, SCRIPT ANALYSIS psa, int[] piX);
publi  static final native int Script reeCache(int psc);
public stati  final native int ScriptGetFont roperties(int hdc, int psc, SCR PT_FONTPROPERTIES sfp);
public  tatic final native int ScriptGe LogicalWidths (SCRIPT_ANALYSIS  sa, int cChars, int cGlyphs, in  piGlyphWidth, int pwLogClust,  nt psva, int[] piDx);
public st tic final native int ScriptItem ze(char[] pwcInChars, int cInCh rs, int cMaxItems, SCRIPT_CONTR L psControl, SCRIPT_STATE psSta e, int pItems, int[] pcItems);
 ublic static final native int S riptJustify(int psva, int piAdv nce, int cGlyphs, int iDx, int  MinKashida, int piJustify);
pub ic static final native int Scri tLayout(int cRuns, byte[] pbLev l, int[] piVisualToLogical, int ] piLogicalToVisual);
public st tic final native int ScriptPlac (int hdc, int psc, int pwGlyphs  int cGlyphs, int psva, SCRIPT_ NALYSIS psa, int piAdvance, int pGoffset, int[] pABC);
public s atic final native int ScriptRec rdDigitSubstitution(int Locale, SCRIPT_DIGITSUBSTITUTE psds);
p blic static final native int Sc iptShape(int hdc, int psc, char ] pwcChars, int cChars, int cMa Glyphs, SCRIPT_ANALYSIS psa, in  pwOutGlyphs, int pwLogClust, i t psva, int[] pcGlyphs);
public static final native int ScriptT xtOut(int hdc, int psc, int x,  nt y, int fuOptions, RECT lprc, SCRIPT_ANALYSIS psa, int pwcRes rved, int iReserved, int pwGlyp s, int cGlyphs, int piAdvance,  nt piJustify, int pGoffset);
pu lic static final native int Scr ptXtoCP(int iX, int cChars, int cGlyphs, int pwLogClust, int ps a, int piAdvance, SCRIPT_ANALYS S psa, int[] piCP, int[] piTrai ing);
public static final nativ  int ScrollWindowEx (int hWnd,  nt dx, int dy, RECT prcScroll,  ECT prcClip, int hrgnUpdate, RE T prcUpdate, int flags);
public static final native int SelectC ipRgn (int hdc, int hrgn);
publ c static final native int Selec Object(int hDC, int HGDIObj);
p blic static final native int Se ectPalette(int hDC, int hpal, b olean bForceBackground);
public static final native int SendInp t (int nInputs, int pInputs, in  cbSize);
public static final n tive int SendMessageW (int hWnd  int Msg, int [] wParam, int [] lParam);
public static final na ive int SendMessageW (int hWnd, int Msg, int [] wParam, int lPa am);
public static final native int SendMessageW (int hWnd, int Msg, int wParam, char [] lParam ;
public static final native in  SendMessageW (int hWnd, int Ms , int wParam, int [] lParam);
p blic static final native int Se dMessageW (int hWnd, int Msg, i