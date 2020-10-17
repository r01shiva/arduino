import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';
import 'package:dio/dio.dart';
import 'package:fluttertoast/fluttertoast.dart';
import 'package:connectivity/connectivity.dart';
void main() {
  runApp(MyApp());
}
class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      theme: ThemeData(
        primarySwatch: Colors.cyan,
      ),
      title: "API Demo",
      home: homepage(),
    );
  }
}

class homepage extends StatefulWidget {
  @override
  _homepageState createState() => _homepageState();
}

class _homepageState extends State<homepage> {

  var temp , hum;
  var response;
  var r1 = 'off';
  var r2 = 'off';
  var r3 = 'off';

  bool _b1 = false,
      _b2 = false,
      _b3 = false;

  bool _visible = false;
  var msg;

  void toast(msg) {
    Fluttertoast.showToast(
        msg: msg,
        toastLength: Toast.LENGTH_SHORT,
        gravity: ToastGravity.BOTTOM,
        timeInSecForIosWeb: 1,
        backgroundColor: Colors.brown,
        textColor: Colors.white,
        fontSize: 16.0
    );
  }

  void _bulb1() async {
    if (r1 == 'off'){
      try {
        toast("loading .....");
        toast("1");
        Response response = await Dio().get('http://192.168.0.164/1/on');
        toast("response");
        setState(() {
          _b1 = true;
          r1 = 'on';
        });
        toast(response.toString());
      } catch (e) {
        toast("catching error");
        toast(e.toString());
      }
    }
    else {
      await Dio().get('http://192.168.0.164/1/off');
      setState(() {
      _b1 = false;
      r1 = 'off';
      });}
  }

  void _bulb2() async {
    if (r2 == 'off'){
      await Dio().get('http://192.168.0.164/2/on');
      setState(() {
        _b2 = true;
        r2 = 'on';
      });}
    else {
      await Dio().get('http://192.168.0.164/2/off');
      setState(() {
        _b2 = false;
        r2 = 'off';
      });}
  }

  void _bulb3() async {
    if (r3 == 'off'){
      await Dio().get('http://192.168.0.164/3/on');
      setState(() {
        _b3 = true;
        r3 = 'on';
      });
    }
    else {
      await Dio().get('http://192.168.0.164/3/off');
      setState(() {
        _b3 = false;
        r3 = 'off';
      });
    }
  }

  void _gethumidity() async {
    Response response = await Dio().get('http://192.168.0.164/humidity');
    toast(response.toString());
    setState(() {
      hum = response;
      _visible = true;
    });
  }

  void _getTemp() async{
    Response response = await Dio().get('http://192.168.0.164/temperature');
    setState(() {
      temp = response;
      _visible = true;
    });
  }
  @override
  Widget build(BuildContext context) {
    // getData();
    return Scaffold(
      appBar: AppBar(
        title: Text(
            "Snap Room"
        ) ,
      ),
      body:Container(
        decoration: BoxDecoration(
        ),
        child: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children:<Widget> [
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[


                  Padding(
                    padding: const EdgeInsets.all(10.0),
                    child: RaisedButton(
                      onPressed: _bulb1,
                      color: _b1?Colors.green:Colors.red,
                      child:Text("Shed Light",
                        style: TextStyle(
                          fontSize: 24.0,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                  Padding(
                    padding: const EdgeInsets.all(10.0),
                    child: RaisedButton(
                      onPressed: _bulb2,
                      color: _b2 ? Colors.green:Colors.red,
                      child:Text("Light",
                        style: TextStyle(
                          fontSize: 24.0,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                  Padding(
                    padding: const EdgeInsets.all(10.0),
                    child: RaisedButton(
                      onPressed: _bulb3,
                      color: _b3?Colors.green:Colors.red,
                      child:Text("Fan",
                        style: TextStyle(
                          fontSize: 24.0,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                ],
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Padding(
                    padding: const EdgeInsets.all(10.0),
                    child: RaisedButton(
                      onPressed: _getTemp,
                      color: Colors.blue,
                      child:Text("Temperature",
                        style: TextStyle(
                          fontSize: 24.0,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                  Padding(
                    padding: const EdgeInsets.all(10.0),
                    child: RaisedButton(
                      onPressed: _gethumidity,
                      color: Colors.blue,
                      child:Text("Humidity",
                        style: TextStyle(
                          fontSize: 24.0,
                          color: Colors.white,
                        ),
                      ),
                    ),
                  ),
                ],
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Visibility(
                    child: Text("$temp",
                      style: TextStyle(
                        fontSize: 14.0,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    visible: _visible,
                  ),
                ],
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Visibility(
                    child: Text("$hum",
                      style: TextStyle(
                        fontSize: 14.0,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    visible: _visible,
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
