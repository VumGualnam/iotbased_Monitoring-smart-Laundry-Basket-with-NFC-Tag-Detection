///////////////////// start: functions /////////////////////////
function setRecommendedTemp(recommended_temp) {
  const triangle = document.getElementById("triangle");
  // const temp_card = document.getElementsByClassName("temp_card");
  let positions = [15, 60, 106, 200]; // pixel as unit
  let text_indicators = ["Cold", "Cold/Warm", "Warm", "Do not wash"]

  // console.log("triangle:datavalue: ", dataValue);
  let positionIndex = recommended_temp; //change this value to 0, 1, 2, or 3 to see the movement of the triangle
  // if (dataValue <= 33) {
  //   positionIndex = 0;
  // } else if (dataValue <= 66) {
  //   positionIndex = 1;
  // } else {
  //   positionIndex = 2;
  // }

  // console.log("positionIndex: " + positionIndex + " | dataValue: " + dataValue);
  // (document.getElementsByClassName("temp-card")[0].textContent.split("\n")[4]).trim()
  triangle.style.bottom = positions[positionIndex] + "px";
  $("#temperature-text").text(text_indicators[positionIndex]);

  if (positionIndex == 3) {
    $("#notification").text(text_indicators[positionIndex]);
    $("#notification").css("color", "red");
    $("#fas-notification").addClass("fas fa-exclamation-triangle");
    $("#fas-notification").css("color", "red");
  }

  if (positionIndex == 0) {
    $("#notification").text(text_indicators[positionIndex]);
    $("#notification").css("color", "green");
    $("#fas-notification").addClass("fas fa-exclamation-triangle");
    $("#fas-notification").css("color", "green");
  }

  // temp_card.style.css = positions[positionIndex] + "px";
}

function BasketFillByData(dataValue) {
  // let dataValue = 5;
  dataValue = Math.ceil(dataValue)
  console.log("dataValue: " + dataValue)
  var defaultBasketLength = document.getElementById("defaultBasketLength").value;
  console.log("defaultBasketLength: " + defaultBasketLength)
  // console.log("typeof(dataValue): " + typeof(dataValue))
  // const basketFill = document.getElementById("basketFill");
  
  var basketImg = document.getElementById("basket");
  var basketImgHeight = basketImg.height;
  console.log("basketImgHeight: " + basketImgHeight);
  
  // let heightP = (dataValue/80) * 100  + "%";
  var heightPercentage = Math.ceil((dataValue/defaultBasketLength) * 100);
  // heightPercentage = parseInt(heightPercentage,10)
  // let heightP = (dataValue/defaultBasketLength) * basketImgHeight;
  // if (dataValue <= 0) {
  //   heightP = "0%";
  // } else if (dataValue <= 4) {
  //   heightP = "4%";
  // } else if (dataValue <= 8) {
  //   heightP = "8%";
  // } else if (dataValue <= 12) {
  //   heightP = "12%";
  // } else if (dataValue <= 16) {
  //   heightP = "16%";
  // } else {
  //   heightP = "20%";
  // }
  // console.log("heightP: " + heightP );

  // basketFill.style.height = heightP + "px";
  var lgstyle = 'linear-gradient( to top, \
      red ' + heightPercentage  + '%, \
      white ' + heightPercentage  + '%, \
      white ' + (100 - heightPercentage) + '%';
  console.log("linear-gradient-style: " + lgstyle);
  $("#basket").css({background: lgstyle});
}

function Notification(message) {
  const messageBox = document.getAnimations("messageBox");

  messageBox.innerHTML = message;
}

function fetchDistanceValue() {
  
  // $.ajax({ 
  //   type : 'GET',
  //   url : "/get_distance_val",
  //   success: function(response){ 
  //     // 'response' is the distance value
  //     BasketFillByData(response);
  //   } 
  // }); 
}


function sendEmail(message){
  email_msg = JSON.stringify({"message": message})
  $.ajax({
    type: "POST",
    url: "/sendEmail",
    data: email_msg,
    contentType: "application/json; charset=utf-8",
    dataType: "json",
    success: function(sendEmail_response) {
      console.log("sendEmail_response: ", sendEmail_response);
      console.log("Sent email: ", email_msg);
    }, 
    failure: function(errMsg) {
      alert(errMsg);
    },
    error: function(error) {
      console.log(error);
    }
  });
}

function getRecommendedTemp(){
  var recommended_temp;
  $.ajax({
    type: "POST",
    url: "/get_temp_val",
    async: false, // this is required to return response data outside ajax call
    success: function(ajax_response) {

      //########## start: To be used only for Testing ########################
      // recommended_temp = 2 // test for "Reminder! ..." email
      // recommended_temp = 3 // test for "Oops! ..." email
      //########## end: To be used only for Testing ########################

      recommended_temp = ajax_response;
      console.log("getRecommendedTemp::ajax_response: ", recommended_temp);
      setRecommendedTemp(recommended_temp); 
    } 
  });
  
  // console.log("getRecommendedTemp::recommended_temp: ", recommended_temp)
  return parseInt(recommended_temp,10); // return in decimal number
}

function isBasketFull(){
  // return boolean
  return document.getElementById("distance_val").value > document.getElementById("defaultBasketLength").value  - 5;
}

///////////////////// end: functions /////////////////////////


