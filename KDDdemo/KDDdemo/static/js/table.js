function loadJSON(callback) {
            var xobj = new XMLHttpRequest();
            xobj.overrideMimeType("text/plain");
            xobj.open('GET', "static/src/papers/results_dm.txt", true);
            xobj.onreadystatechange = function () {
              if (xobj.readyState == 4 && xobj.status == "200") {
                callback(xobj.responseText);
              }
            };
            xobj.send(null);
}
function loadJSON2(callback) {
            var xobj = new XMLHttpRequest();
            xobj.overrideMimeType("text/plain");
            xobj.open('GET', "static/src/papers/results2_dm.txt", true);
            xobj.onreadystatechange = function () {
              if (xobj.readyState == 4 && xobj.status == "200") {
                callback(xobj.responseText);
              }
            };
            xobj.send(null);
}
loadJSON(function(json) {
loadJSON2(function(json2) {
var table1 = json;
var table2 = json2;
//So far, table1 and table2 are stored in string. We need convert them to be dictionary the following steps did this.
//For table1
var table1_array = table1.split("\n");
var table1_dict = {};
for (var i = 0; i < table1_array.length-1; i++) {
  if (table1_array[i] == "") {
    continue;
  }
  else {
    if (table1_array[i].includes(":")) {
      table1_dict[table1_array[i].substr(0,table1_array[i].length - 1)]= table1_array[i+1].split(" ");
    }
  }
}

//For table2
var table2_array = table2.split("\n");
var table2_dict = {};
for (var i = 0; i < table2_array.length - 1; i++) {
  if (table2_array[i] == "") {
    continue;
  }
  else {
    if (table2_array[i][table2_array[i].length - 1] == ":") {
      var key = table2_array[i].substr(0,table2_array[i].length - 1);
      if (!(key in table2_dict)) {
        table2_dict[key] = [];
        table2_dict[key].push(table2_array[i+1]);
      }
      else {
      table2_dict[key].push(table2_array[i+1]);
      }
    }
  }
}
for (var key in table2_dict) {
  for (var i = 0; i < table2_dict[key].length; i++) {
    table2_dict[key][i] = table2_dict[key][i].split(":\t");
  }
}
console.log(table2_dict);
//So far, we have got all the tables, we begin to form the table1
var keyword_title = document.getElementById("keyword_title");
var keyword_title_html = "";
keyword_title_html += "<tr>";
for (var key in table1_dict) {
  keyword_title_html += "<th>" + key.toString() + "</th>";
}
keyword_title_html += "</tr>";
keyword_title.innerHTML = keyword_title_html;

var keyword_table = document.getElementById("keyword_table");
var keyword_table_html = "";
for (var i = 0; i < 10; i++) {
  keyword_table_html += "<tr>";
  for (var key in table1_dict) {
    keyword_table_html += "<td>";
    keyword_table_html += table1_dict[key][i];
    keyword_table_html += "</td>";
  }
  keyword_table_html += "</tr>";
}
keyword_table.innerHTML = keyword_table_html;


//Now, we need to complete table2
var datatable_title = document.getElementById("datatable_title");
var datatable_title_html= "";
datatable_title_html += "<tr>";
datatable_title_html += "<th>Range of k</th>";
for (var key in table2_dict) {
  datatable_title_html += "<th>";
  datatable_title_html += key.toString();
  datatable_title_html += "</th>";
}
datatable_title_html += "</tr>";
datatable_title.innerHTML = datatable_title_html;

var datatable_body = document.getElementById("datatable_body");
var datatable_body_html = "";
var firstkey = Object.keys(table2_dict)[0];
var templength = table2_dict[firstkey].length;
for (var i = 0; i < templength; i++) {
  datatable_body_html += "<tr>";
  datatable_body_html += "<td>";
  datatable_body_html += table2_dict[firstkey][i][0].replace("<", "&#60");
  console.log(table2_dict[firstkey][i][0]);
  datatable_body_html += "</td>";
  for (var key in table2_dict) {
    datatable_body_html += "<td>";
    temp_list = table2_dict[key][i][1].split(" ");
    for (var j = 0; j < 5; j++) {
      datatable_body_html += temp_list[j] +"<br/>"+ " ";
    }
    datatable_body_html += "</td>";
  }
  datatable_body_html += "</tr>";
}
datatable_body.innerHTML = datatable_body_html;
});
});
