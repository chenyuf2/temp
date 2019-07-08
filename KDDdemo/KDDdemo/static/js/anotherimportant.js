// jQuery.get('static/src/papers/emb_dm_w_tmp.txt',function(data){console.log(data);});
function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

window.onload = function () {
         //Make sure the function fires as soon as the page is loaded
        setInterval(importantjs, 3500); //Then set it to run again after 0.1s

    }
function importantjs() {
  // console.log("why it doesn't work")
  // console.log('successfully loading materials');
  jQuery.get('static/src/papers/topics_dm.txt',function(data3){
    jQuery.get('static/src/papers/emb_dm_w_tmp.txt',function(data){
        jQuery.get('static/js/loadingornot.txt',function(data2){
                  

  var table3 = data;
  var test = data2;
  // console.log(document.getElementById("useful").innerHTML);
  console.log(data2);
  console.log(data);
  //table3_array stores the dynamic values
  // test stores if the program begins to run
  var table3_array = table3.split("\n");
  var test_array = test.split("\n");
  var loading = test_array[0];
  // console.log(loading);
  if (loading == "false") {
    // show table results
    ids = ["mainNav", "section1", "section2", "services", "portfolio", "foots"];
    for (var i = 0; i < ids.length; i++) {
      document.getElementById(ids[i]).style.display = "block";
    }
    document.getElementById("loader1").style.display = "none";
    var page = document.getElementById("page-top");
    page.style.backgroundColor="white";
      document.getElementById("loader2").style.display = "none";
      var useful =document.getElementById("useful");
      useful.innerHTML = "1";
      // console.log("god");
  } else {
    // if (document.getElementById("useful").innerHTML == "1" && document.getElementById("mainNav").style.display == "block") {
    //   sleep(3000);
    // }
 //hide table results
 ids = ["mainNav", "section1", "section2", "services", "portfolio", "foots"];
 for (var i = 0; i < ids.length; i++) {
   document.getElementById(ids[i]).style.display = "none";
 }
 document.getElementById("loader1").style.display = "block";
   document.getElementById("loader2").style.display = "block";
   var page = document.getElementById("page-top");
   page.style.backgroundColor="gold";

   var useful = document.getElementById("useful");
   var old_num = parseInt(useful.innerHTML);
   var user_input = data3;
   var user_input_array = user_input.split("\n");
   var num_of_input = user_input_array.length - 1;
   var dynamic = data;
   var dynamic_array = data.split("\n");
   // console.log(old_num);
   if (old_num < dynamic_array.length) {
     for (var i = old_num-1; i < dynamic_array.length-1; i++) {
       var temp = dynamic_array[i];
       $('#loader2').append('<div class="box" style="text-align:center;"><span>'+temp+'</span></div>');
     }
     useful.innerHTML = dynamic_array.length.toString();
   } else {
     return;
   }

  }

  //So far, table1 and table2 are stored in string. We need convert them to be dictionary the following steps did this.
  //For table1
  // var table1_array = table1.split("\n");
  // var table1_dict = {};
  // for (var i = 0; i < table1_array.length-1; i++) {
  //   if (table1_array[i] == "") {
  //     continue;
  //   }
  //   else {
  //     if (table1_array[i].includes(":")) {
  //       table1_dict[table1_array[i].substr(0,table1_array[i].length - 1)]= table1_array[i+1].split(" ");
  //     }
  //   }
  // }
  //
  // //For table2
  // var table2_array = table2.split("\n");
  // var table2_dict = {};
  // for (var i = 0; i < table2_array.length - 1; i++) {
  //   if (table2_array[i] == "") {
  //     continue;
  //   }
  //   else {
  //     if (table2_array[i][table2_array[i].length - 1] == ":") {
  //       var key = table2_array[i].substr(0,table2_array[i].length - 1);
  //       if (!(key in table2_dict)) {
  //         table2_dict[key] = [];
  //         table2_dict[key].push(table2_array[i+1]);
  //       }
  //       else {
  //       table2_dict[key].push(table2_array[i+1]);
  //       }
  //     }
  //   }
  // }
  // for (var key in table2_dict) {
  //   for (var i = 0; i < table2_dict[key].length; i++) {
  //     table2_dict[key][i] = table2_dict[key][i].split(":\t");
  //   }
  // }
  //
  // //So far, we have got all the tables, we begin to form the table1
  // var keyword_title = document.getElementById("keyword_title");
  // var keyword_title_html = "";
  // // keyword_title_html += "<tr>";
  // // for (var key in table1_dict) {
  // //   keyword_title_html += "<th>" + key.toString() + "</th>";
  // // }
  // // keyword_title_html += "</tr>";
  // // keyword_title.innerHTML = keyword_title_html;
  //
  // var keyword_table = document.getElementById("keyword_table");
  // var keyword_table_html = "";
  // // for (var i = 0; i < 10; i++) {
  // //   keyword_table_html += "<tr>";
  // //   for (var key in table1_dict) {
  // //     keyword_table_html += "<td>";
  // //     keyword_table_html += table1_dict[key][i];
  // //     keyword_table_html += "</td>";
  // //   }
  // //   keyword_table_html += "</tr>";
  // // }
  //   //Now we need to add stuff to the table1
  // var added_table = table3.split("\n");
  // var num_of_input = Object.keys(table1_dict).length;
  // keyword_title_html += "<tr>";
  // for (var i = 0; i < num_of_input; i++) {
  //   keyword_title_html += "<th>";
  //   keyword_title_html += added_table[i];
  //   keyword_title_html += "</th>";
  // }
  // keyword_table_html += "</tr>";
  // keyword_title.innerHTML = keyword_title_html;
  // for (var i = num_of_input; i < added_table.length; i++) {
  //   if ((i+1)%num_of_input ==1) {
  //     keyword_table_html += "<tr>"
  //   }
  //   keyword_table_html += "<td>";
  //   keyword_table_html += added_table[i].split(" ")[1];
  //   keyword_table_html += "</td>";
  //   if ((i+1)%num_of_input == 0) {
  //     keyword_table_html += "</tr>";
  //   }
  // }
  // console.log(added_table);
  // keyword_table.innerHTML = keyword_table_html;
  //
  //
  // //Now, we need to complete table2
  // var datatable_title = document.getElementById("datatable_title");
  // var datatable_title_html= "";
  // datatable_title_html += "<tr>";
  // datatable_title_html += "<th>Range of k</th>";
  // for (var key in table2_dict) {
  //   datatable_title_html += "<th>";
  //   datatable_title_html += key.toString();
  //   datatable_title_html += "</th>";
  // }
  // datatable_title_html += "</tr>";
  // datatable_title.innerHTML = datatable_title_html;
  //
  // var datatable_body = document.getElementById("datatable_body");
  // var datatable_body_html = "";
  // var firstkey = Object.keys(table2_dict)[0];
  // var templength = table2_dict[firstkey].length;
  //
  // for (var i = 0; i < templength; i++) {
  //   datatable_body_html += "<tr>";
  //   datatable_body_html += "<td>";
  //   datatable_body_html += table2_dict[firstkey][i][0].replace("<", "&#60");
  //   datatable_body_html += "</td>";
  //   for (var key in table2_dict) {
  //     datatable_body_html += "<td>";
  //     temp_list = table2_dict[key][i][1].split(" ");
  //     for (var j = 0; j < 5; j++) {
  //       datatable_body_html += temp_list[j] +"<br/>"+ " ";
  //     }
  //     datatable_body_html += "</td>";
  //   }
  //   datatable_body_html += "</tr>";
  // }
  // datatable_body.innerHTML = datatable_body_html;

  //Now we need to add stuff to the table1
});
});
});

}
