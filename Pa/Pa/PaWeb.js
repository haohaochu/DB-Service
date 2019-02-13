"use strict";
var webpage = require('webpage');
var page = webpage.create();
var fs = require('fs');
var system = require('system');
var posturl;
var url;

url = system.args[1];
posturl = system.args[2];

page.settings.userAgent = 'SpecialAgent';
page.settings.resourceTimeout = 30000; // 5 seconds
page.onResourceTimeout = function(e) 
{
  console.log(e.errorCode);   // it'll probably be 408 
  console.log(e.errorString); // it'll probably be 'Network timeout on resource'
  console.log(e.url);         // the url whose request timed out
  phantom.exit(1);
};

page.open(url, function(status) 
{
  if (status !== 'success') 
  {
    console.log('001 URL Access Error\n');
	phantom.exit();
  } 
  
  var ua = page.evaluate(function() 
  {
	return document.body.innerHTML;
  });
  
  if( ua.length == 0 )
  {
	console.log('003 Page Empty\n');
	phantom.exit();
  }

  
  //console.log('paweb success');
//  page.openUrl(posturl, {operation: 'post', data: ua, encoding: "utf-8"}, page.settings, function(status) 
//  {
//	 if(status !== "success") 
//	 {
//	     console.log('002 URL Post Error\n');
//		 phantom.exit();
//	 }
	 
//     console.log('000 Success\n');
//	 phantom.exit();
// });	

  var settings = {
	operation: 'POST',
	encoding: 'utf8',
	data: ua
  };
  
  page.open(posturl, settings, function(status) 
  {
	 if(status !== "success") 
	 {
	     console.log('002 URL Post Error\n');
		 phantom.exit();
	 }
   
	 console.log('000 Success\n');
	 phantom.exit();
  });
});



