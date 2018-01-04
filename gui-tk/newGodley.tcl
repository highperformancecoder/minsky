proc newOpenGodley {id} {
    if {![ winfo exists ".$id"]} {
        image create cairoSurface $id -surface $id
        toplevel .$id
        label .$id.table -image $id -width 500 -height 100
        pack .$id.table -fill both -expand 1
        bind .$id.table <Configure> "$id.requestRedraw"
        bind .$id.table <Destroy> "$id.delete"
    }
    wm deiconify .$id
}
