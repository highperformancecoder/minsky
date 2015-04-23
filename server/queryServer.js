var connection;

var connect=function() 
{
    connection=new WebSocket(document.getElementById("server").value);
    connection.onopen = function() 
    {
        // kick of requests for version, commands and payloads
        connection.send('{"msg":"version"}');
        connection.send('{"msg":"commands"}');
        connection.send('{"msg":"payloads"}');
    };

    connection.onmessage = function(m) 
    {
//        document.write(m.data+"<br>");
        var response = JSON.parse(m.data);
//        document.write(response.msg+"<br>");
        switch (response.msg)
        {
        case "version":
            var verElement=document.getElementById("version");
            var v=response.payload;
            verElement.innerHTML="Schema "+v.schemaVersion+
                ", Minsky "+v.minskyVersion+", EcoLab "+v.ecolabVersion;
            break;
        case "commands":
            var c=response.payload;
            var select=document.getElementById("commands");
            for (var i=0; i<c.length; ++i)
                {
                    var option=document.createElement("option");
                    option.appendChild(document.createTextNode(c[i]));
                    select.appendChild(option);
                }
            break;
        case "payloads":
            var c=response.payload;
            var select=document.getElementById("payloads");
            for (var i=0; i<c.length; ++i)
                {
                    var option=document.createElement("option");
                    option.appendChild(document.createTextNode(c[i]));
                    select.appendChild(option);
                }
            break;
        }
    };

    connection.onerror = function(e)
    {
        alert("Websocket error"+e);
    };
}

var execute=function() 
{
    // display return result
    connection.onmessage = function(m)
    {
        document.getElementById("response").innerHTML=m.data;
    }

    var msg={};
    msg.msg=document.getElementById("commands").value;
    msg.payloadClass=document.getElementById("payloads").value;
    if (connection) connection.send(JSON.stringify(msg));
}
