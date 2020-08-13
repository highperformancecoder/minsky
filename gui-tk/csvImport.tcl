# assume canvas.minsky.item is the variable

proc CSVImportDialog {} {
    if {[llength [info commands minsky.canvas.item.valueId]]==0} return
    getValue [minsky.canvas.item.valueId]
    global workDir csvParms
    if {![winfo exists .wiring.csvImport]} {
        toplevel .wiring.csvImport

        # file/url control
        frame .wiring.csvImport.fileUrl
        button .wiring.csvImport.fileUrl.file -text "File" -command {
            set csvParms(url) [tk_getOpenFile -filetypes {{CSV {.csv}} {All {.*}}} -initialdir $workDir]
            .wiring.csvImport.fileUrl.load invoke
            raise .wiring.csvImport
        }
        label .wiring.csvImport.fileUrl.orUrl -text "or URL"
        entry .wiring.csvImport.fileUrl.url -textvariable csvParms(url) -width 100
        button .wiring.csvImport.fileUrl.load -text "Load" -command {
            minsky.value.csvDialog.url $csvParms(url)
            minsky.value.csvDialog.loadFile
            minsky.value.csvDialog.requestRedraw
        }
        bind .wiring.csvImport.fileUrl.url <Key-Return> ".wiring.csvImport.fileUrl.load invoke"
        
        pack .wiring.csvImport.fileUrl.file .wiring.csvImport.fileUrl.orUrl \
            .wiring.csvImport.fileUrl.url .wiring.csvImport.fileUrl.load  -side left
        pack .wiring.csvImport.fileUrl

        # delimiters control
        frame .wiring.csvImport.delimiters
        label .wiring.csvImport.delimiters.columnarLabel -text "Columnar"
        ttk::checkbutton .wiring.csvImport.delimiters.columnar -variable csvParms(columnar) -command {
            minsky.value.csvDialog.spec.columnar $csvParms(columnar)
            minsky.value.csvDialog.requestRedraw
        }
        label .wiring.csvImport.delimiters.separatorLabel -text Separator
        ttk::combobox .wiring.csvImport.delimiters.separatorValue -values {
            "," ";" "<tab>" "<space>"} -textvariable csvParms(separator) -width 5
        bind .wiring.csvImport.delimiters.separatorValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.separator $csvParms(separator)}
        label .wiring.csvImport.delimiters.decSeparatorLabel -text "Decimal Separator"
        ttk::combobox .wiring.csvImport.delimiters.decSeparatorValue -values {
            "." ","} -textvariable csvParms(decSeparator) -width 5
        bind .wiring.csvImport.delimiters.decSeparatorValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.decSeparator $csvParms(decSeparator)}
        label .wiring.csvImport.delimiters.escapeLabel -text Escape
        ttk::combobox .wiring.csvImport.delimiters.escapeValue -values {
            "\\"} -textvariable csvParms(escape) -width 5
        bind .wiring.csvImport.delimiters.escapeValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.escape $csvParms(escape)}
        label .wiring.csvImport.delimiters.quoteLabel -text Quote
        ttk::combobox .wiring.csvImport.delimiters.quoteValue -values {
            "'" "\"" } -textvariable csvParms(quote) -width 5
        bind .wiring.csvImport.delimiters.quoteValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.quote $csvParms(quote)}
        label .wiring.csvImport.delimiters.mergeLabel -text "Merge Delimiters"
        ttk::checkbutton .wiring.csvImport.delimiters.mergeValue -variable csvParms(mergeDelimiters) -command {
            minsky.value.csvDialog.spec.mergeDelimiters $csvParms(mergeDelimiters)
        }
        label .wiring.csvImport.delimiters.missingLabel -text "Missing Value"
        ttk::combobox .wiring.csvImport.delimiters.missingValue \
            -textvariable csvParms(missingValue) -values {nan 0} -width 5
        bind .wiring.csvImport.delimiters.missingValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.missingValue $csvParms(missingValue)}

        label .wiring.csvImport.delimiters.colWidthLabel -text "Col Width"
        spinbox .wiring.csvImport.delimiters.colWidth -from 10 -to 500 -increment 10 \
            -width 5 -command {adjustColWidth %s} -validate key -validatecommand {adjustColWidth %P}
        .wiring.csvImport.delimiters.colWidth set 50
        
        pack .wiring.csvImport.delimiters.columnarLabel .wiring.csvImport.delimiters.columnar \
            .wiring.csvImport.delimiters.separatorLabel .wiring.csvImport.delimiters.separatorValue \
            .wiring.csvImport.delimiters.decSeparatorLabel .wiring.csvImport.delimiters.decSeparatorValue \
            .wiring.csvImport.delimiters.escapeLabel .wiring.csvImport.delimiters.escapeValue \
            .wiring.csvImport.delimiters.quoteLabel .wiring.csvImport.delimiters.quoteValue \
            .wiring.csvImport.delimiters.mergeLabel .wiring.csvImport.delimiters.mergeValue \
            .wiring.csvImport.delimiters.missingLabel .wiring.csvImport.delimiters.missingValue \
            .wiring.csvImport.delimiters.colWidthLabel .wiring.csvImport.delimiters.colWidth -side left

        pack .wiring.csvImport.delimiters

        # horizontal dimension control
        frame .wiring.csvImport.horizontalName
        label .wiring.csvImport.horizontalName.text -text "Horizontal dimension"
        entry .wiring.csvImport.horizontalName.value -width 30 -textvariable csvParms(horizontalDimension)
        label .wiring.csvImport.horizontalName.duplicateKeyLabel -text "Duplicate Key Action"
        ttk::combobox .wiring.csvImport.horizontalName.duplicateKeyValue \
            -textvariable csvParms(duplicateKeyValue) \
            -values {throwException sum product min max av} -width 15
        bind .wiring.csvImport.horizontalName.duplicateKeyValue <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.duplicateKeyAction $csvParms(duplicateKeyValue)}
        label .wiring.csvImport.horizontalName.typeLabel -text "Type"
        ttk::combobox .wiring.csvImport.horizontalName.type \
            -textvariable csvParms(horizontalType) \
            -values {string value time} -width 15 -state readonly
        bind .wiring.csvImport.horizontalName.type <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.horizontalDimension.type $csvParms(horizontalType)
            if {$csvParms(horizontalType)=="time"} {
                .wiring.csvImport.horizontalName.format configure -values $timeFormatStrings
            } else {
                .wiring.csvImport.horizontalName.format configure -values ""
            }
        }
        label .wiring.csvImport.horizontalName.formatLabel -text "Format"
        ttk::combobox .wiring.csvImport.horizontalName.format \
            -textvariable csvParms(horizontalFormat) -width 15 
        bind .wiring.csvImport.horizontalName.format <<ComboboxSelected>> {
            minsky.value.csvDialog.spec.horizontalDimension.units $csvParms(horizontalFormat)}

        pack .wiring.csvImport.horizontalName.duplicateKeyLabel .wiring.csvImport.horizontalName.duplicateKeyValue -side left
        pack .wiring.csvImport.horizontalName.text .wiring.csvImport.horizontalName.value -side left
        pack .wiring.csvImport.horizontalName
        pack .wiring.csvImport.horizontalName.typeLabel .wiring.csvImport.horizontalName.type .wiring.csvImport.horizontalName.formatLabel .wiring.csvImport.horizontalName.format -side left
        
        image create cairoSurface csvDialogTable -surface minsky.value.csvDialog
        label .wiring.csvImport.table -image csvDialogTable -width 800 -height 300
        pack .wiring.csvImport.table -fill both -expand 1 -side top

        scale .wiring.csvImport.hscroll -orient horiz -from -100 -to 1000 -showvalue 0 -command scrollTable
        pack .wiring.csvImport.hscroll -fill x -expand 1 -side top
        
        buttonBar .wiring.csvImport {}
        # redefine OK command to not delete the the import window on error
        global csvImportFailed
        set csvImportFailed 0
        .wiring.csvImport.buttonBar.ok configure -command csvImportDialogOK -text Import
        bind .wiring.csvImport.table <Configure> "minsky.value.csvDialog.requestRedraw"
        bind .wiring.csvImport.table <Button-1> {csvImportButton1 %x %y};
        bind .wiring.csvImport.table <ButtonRelease-1> {csvImportButton1Up %x %y %X %Y};
        bind .wiring.csvImport.table <B1-Motion> {
            minsky.value.csvDialog.xoffs [expr $csvImportPanX+%x];
            if $movingHeader {
                set row [minsky.value.csvDialog.rowOver %y]
                if {$row>=4} {minsky.value.csvDialog.spec.headerRow [expr $row-4]}
                minsky.value.csvDialog.flashNameRow [expr $row==3]
            }
            minsky.value.csvDialog.requestRedraw
        }
    }
    
    if [string length [minsky.value.csvDialog.url]] {
        set workDir [file dirname [minsky.value.csvDialog.url]]
    }
    set csvParms(url) [minsky.value.csvDialog.url]
    set csvParms(separator) [minsky.value.csvDialog.spec.separator]
    set csvParms(decSeparator) [minsky.value.csvDialog.spec.decSeparator]
    set csvParms(escape) [minsky.value.csvDialog.spec.escape]
    set csvParms(quote) [minsky.value.csvDialog.spec.quote]
    set csvParms(mergeDelimiters) [minsky.value.csvDialog.spec.mergeDelimiters]
    set csvParms(missingValue) [minsky.value.csvDialog.spec.missingValue]
    set csvParms(duplicateKeyValue) [minsky.value.csvDialog.spec.duplicateKeyAction]
    set csvParms(horizontalDimension) [minsky.value.csvDialog.spec.horizontalDimName]
    .wiring.csvImport.delimiters.colWidth set [minsky.value.csvDialog.colWidth]
    wm deiconify .wiring.csvImport
    raise .wiring.csvImport
    minsky.value.csvDialog.requestRedraw
}

proc csvImportDialogOK {} {
    global csvParms
    minsky.value.csvDialog.spec.horizontalDimName $csvParms(horizontalDimension)
    set csvImportFailed [catch {loadVariableFromCSV minsky.value.csvDialog.spec "$csvParms(url)"} err]	
    if $csvImportFailed {
        toplevel .csvImportError
        label .csvImportError.errMsg -text $err
        label .csvImportError.msg -text "Would you like to generate a report?"
        pack .csvImportError.errMsg .csvImportError.msg -side top
        buttonBar .csvImportError "doReport {$filename}"                
    } else {
        catch reset
        cancelWin .wiring.csvImport
    }
}

proc doReport {inputFname} {
    global workDir
    set fname [tk_getSaveFile -initialfile [file rootname $inputFname]-error-report.csv -initialdir $workDir]
    if [string length $fname] {
        eval minsky.value.csvDialog.reportFromFile {$inputFname} {$fname}
    }
}

        
proc scrollTable v {
    minsky.value.csvDialog.xoffs [expr -$v]
    minsky.value.csvDialog.requestRedraw
}

proc adjustColWidth {w} {
    minsky.value.csvDialog.colWidth $w
    minsky.value.csvDialog.requestRedraw
    return 1
}

proc csvImportButton1 {x y} {
    global csvImportPanX mouseSave movingHeader
    set csvImportPanX [expr [minsky.value.csvDialog.xoffs]-$x]
    set movingHeader [expr [minsky.value.csvDialog.rowOver $y]-4 == [minsky.value.csvDialog.spec.headerRow]]
    set mouseSave "$x $y"
}

proc closeCombo setter {
    eval $setter \[.wiring.csvImport.text.combo get\]
    wm withdraw .wiring.csvImport.text
    minsky.value.csvDialog.requestRedraw
}

proc setupCombo {getter setter title configure X Y} {
    wm title .wiring.csvImport.text $title
    eval .wiring.csvImport.text.combo configure $configure
    .wiring.csvImport.text.combo set $getter
    bind .wiring.csvImport.text.combo <<ComboboxSelected>> "closeCombo $setter"
    bind .wiring.csvImport.text.combo <Return> "closeCombo $setter"
    wm deiconify .wiring.csvImport.text
    wm geometry .wiring.csvImport.text +$X+$Y
    raise .wiring.csvImport.text 
}

proc csvImportButton1Up {x y X Y} {
    global mouseSave csvParms movingHeader
    # combobox used for setting dimension type
    if {![winfo exists .wiring.csvImport.text.combo]} {
        toplevel .wiring.csvImport.text
        ttk::combobox .wiring.csvImport.text.combo -values {"string" "value" "time"} -state readonly
        pack .wiring.csvImport.text.combo
        wm withdraw .wiring.csvImport.text
    }

    set col [minsky.value.csvDialog.columnOver $x]
    set row [minsky.value.csvDialog.rowOver $y]
    if {abs([lindex $mouseSave 0]-$x)==0 && abs([lindex $mouseSave 1]-$y)==0} {
        # mouse click - implement dialog interaction logic
        switch [minsky.value.csvDialog.rowOver $y] {
            0 {minsky.value.csvDialog.spec.toggleDimension $col
                minsky.value.csvDialog.requestRedraw
            }
            1 {if {$col<[minsky.value.csvDialog.spec.dimensions.size]} {
                setupCombo [[minsky.value.csvDialog.spec.dimensions.@elem $col].type] \
                    "minsky.value.csvDialog.spec.dimensions($col).type" \
                    "Dimension type" {-values {"string" "value" "time"} -state readonly} $X $Y
            }}
            2 {if {$col<[minsky.value.csvDialog.spec.dimensions.size]} {
                set units "{}"
                if {[[minsky.value.csvDialog.spec.dimensions.@elem $col].type]=="time"} {
                    global timeFormatStrings
                    set units "\{[set timeFormatStrings]\}"
                }
                setupCombo [[minsky.value.csvDialog.spec.dimensions.@elem $col].units] \
                    "minsky.value.csvDialog.spec.dimensions($col).units" \
                    "Dimension units/format" "-values $units -state normal" $X $Y
            }}
            3 {if {$col<[minsky.value.csvDialog.spec.dimensions.size]} {
                setupCombo [[minsky.value.csvDialog.spec.dimensionNames.@elem $col]] \
                    "minsky.value.csvDialog.spec.dimensionNames($col)" \
                    "Dimension name" "-values \"[minsky.value.csvDialog.headerForCol $col]\" -state normal" $X $Y
            }}
            default {
                minsky.value.csvDialog.spec.setDataArea [expr $row-4] $col
                minsky.value.csvDialog.requestRedraw
            }
        }
    }
    if {$movingHeader && $row==3} {
        # copy columns headers to dimension names
        set oldHeaderRow [expr [minsky.value.csvDialog.rowOver [lindex $mouseSave 1]]-4]
        minsky.value.csvDialog.copyHeaderRowToDimNames $oldHeaderRow
        minsky.value.csvDialog.spec.headerRow $oldHeaderRow
        minsky.value.csvDialog.flashNameRow 0
        minsky.value.csvDialog.requestRedraw
    }
    set movingHeader 0
}
