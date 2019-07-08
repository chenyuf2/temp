// jQuery.get('static/src/papers/emb_dm_w_tmp.txt',function(data){console.log(data);});

function helper(inputstr, category) {

       $('#rowlength').append('<div class="portfolio-item" style="width: 500px!important; display:inline-block;margin-right:-4px;"><div class=" portfolio-caption box" style="border-radius:20px;"><h4>'+inputstr+'</h4><p class="text-muted">'+category+'</p></div></div>');

}


window.onload = function () {
         //Make sure the function fires as soon as the page is loaded
        setInterval(importantjs, 4000); //Then set it to run again after 0.1s

    }
function importantjs() {
  // console.log("why it doesn't work")
  // console.log('successfully loading materials');
    jQuery.get('static/js/loadingornot2.txt',function(data2){
    jQuery.get('static/src/papers/emb_dm_w_tmp.txt',function(data){

                  jQuery.get('static/src/papers/topics_dm.txt',function(data3){
                    console.log(data2);

                      var test = data2;
                    console.log(data);
                      console.log(document.getElementById("useful").innerHTML);
                      //table3_array stores the dynamic values
                      // test stores if the program begins to run

                      var test_array = test.split("\n");
                      var loading = test_array[0];
                      // console.log(loading);
                      if (loading == "false") {
                        // show table results
                        ids = ["mainNav", "section1", "section2", "services", "portfolio2", "foots"];
                        for (var i = 0; i < ids.length; i++) {
                          document.getElementById(ids[i]).style.display = "block";
                        }
                        document.getElementById("loader1").style.display = "none";
                        var page = document.getElementById("page-top");
                        page.style.backgroundColor="white";
                          document.getElementById("loader2").style.display = "none";
                            document.getElementById("portfolio").style.display = "none";
                          var useful =document.getElementById("useful");
                          useful.innerHTML = "1";
                          // console.log("god");
                      } else {
                        // if (document.getElementById("useful").innerHTML == "1" && document.getElementById("mainNav").style.display == "block") {
                        //   sleep(3000);
                        // }
                     //hide table results

                     ids = ["mainNav", "section1", "section2", "services", "portfolio2", "foots"];
                     for (var i = 0; i < ids.length; i++) {
                       document.getElementById(ids[i]).style.display = "none";
                     }
                     document.getElementById("loader1").style.display = "block";
                       document.getElementById("loader2").style.display = "block";
                         document.getElementById("portfolio").style.display = "block";
                       var page = document.getElementById("page-top");
                    page.style.backgroundColor="#DCC7AA";

                       var useful = document.getElementById("useful");
                       var old_num = parseInt(useful.innerHTML);
                       var user_input = data3;
                       var user_input_array = user_input.split("\n");
                       var num_of_input = user_input_array.length - 1;
                       var dynamic = data;
                       var dynamic_array = data.split("\n");
                       if (dynamic_array.length > 1) {
                         var rowlength = document.getElementById("rowlength");
                         rowlength.style.width = (500*num_of_input).toString() + "px";
                       }
                       console.log(dynamic_array.length);
                       console.log(old_num);
                       // console.log(old_num);
                       if (old_num < dynamic_array.length) {
                         for (var i = old_num-1; i < dynamic_array.length-1; i++) {
                           var temp_array = dynamic_array[i].split(" ");
                           var temp = temp_array[temp_array.length - 2];
                           if (i >= num_of_input) {
                             var category = user_input_array[i%num_of_input];
                             helper(temp, category.toUpperCase());
                           } else {
                           // console.log(dynamic_array[i].split(" "));
                           if (i < num_of_input) {
                             helper(temp.toUpperCase(), "");
                           } else {
                        helper(temp, "");
                      }
                    }
                         }
                         var current_loading = document.getElementById("loadingnum");
                         var current_loading_num = parseInt(current_loading.innerHTML);
                         var percentage = Math.floor((dynamic_array.length-1)*100/(num_of_input * 2));
                         if (current_loading_num <= percentage) {
                           for (var  i = current_loading_num; i < percentage; i++) {
                             current_loading.innerHTML = i.toString() + "%";
                           }
                         }
                         useful.innerHTML = dynamic_array.length.toString();
                       } else {
                         useful.innerHTML = dynamic_array.length.toString();
                         return;
                       }

                      }

});
});
});

}
