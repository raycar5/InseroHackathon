package com.tadaacar;

import android.content.Context;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.util.Pair;
import android.view.Gravity;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.NumberPicker;
import android.widget.TextView;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

public class MainActivity extends AppCompatActivity {
    boolean driving = false;
    TextView drivingT;
    UUID pebbleApiKey = UUID.fromString("17466bab-b221-489f-b07b-81d45fe2027a");
    String numberPlate = "AB 12345";
    ArrayList<String> bookingsKm = new ArrayList<>();
    String tempKm = null;
    ArrayList<Calendar> bookingsSD = new ArrayList<>();
    Calendar tempSD = null;
    ArrayList<Calendar> bookingsED = new ArrayList<>();
    Calendar tempED = null;
    NumberPicker scoreNP;
    int score = 0;
    CheckBox doorsOpenCB;
    boolean doorsOpen;
    CheckBox keyInCB;
    boolean keyIn;
    CheckBox chargingCB;
    boolean charging;
    CheckBox geoFenceCB;
    boolean geoFence;
    FloatingActionButton fab;

    LinearLayout bKML;
    LinearLayout bSDL;
    LinearLayout bEDL;

    private void getElementsById(){
        scoreNP = (NumberPicker) findViewById(R.id.score);
        scoreNP.setMaxValue(100);
        scoreNP.setMinValue(4);
        doorsOpenCB = (CheckBox) findViewById(R.id.doorsOpen);
        keyInCB = (CheckBox) findViewById(R.id.keyIn);
        chargingCB = (CheckBox) findViewById(R.id.charging);
        geoFenceCB = (CheckBox) findViewById(R.id.geoFence);
        bKML = (LinearLayout) findViewById(R.id.bookingsKm);
        bSDL = (LinearLayout) findViewById(R.id.bookingsStart);
        bEDL = (LinearLayout) findViewById(R.id.bookingsEnd);
        fab = (FloatingActionButton) findViewById(R.id.fab);
        drivingT=(TextView) findViewById(R.id.status);

    }
    private void startListeners(){
        scoreNP.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                score = newVal;
            }
        });
        doorsOpenCB.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                doorsOpen = isChecked;
            }
        });
        keyInCB.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                keyIn = isChecked;
            }
        });
        chargingCB.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                charging = isChecked;
            }
        });
        geoFenceCB.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                geoFence = isChecked;
            }
        });
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                bookingsKm.add("20-50");
                Calendar cal1 = Calendar.getInstance();
                cal1.add(Calendar.HOUR, -5);
                bookingsSD.add(cal1);
                Calendar cal = Calendar.getInstance();
                cal.add(Calendar.HOUR, 5);
                bookingsED.add(cal);
                renderTables();
            }
        });
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getElementsById();
        startListeners();
        PebbleKit.registerReceivedDataHandler(getApplicationContext(), dataReceiver);

    }
    public void sendMessageToPebble(byte type,long data){
        PebbleDictionary dict = new PebbleDictionary();
        dict.addUint8(0,type);
        dict.addUint32(1, (int) data);
        PebbleKit.sendDataToPebble(getApplicationContext(),pebbleApiKey,dict);
    }
    PebbleKit.PebbleDataReceiver dataReceiver = new PebbleKit.PebbleDataReceiver(pebbleApiKey) {
        @Override
        public void receiveData(Context context, int transaction_id,
                                PebbleDictionary dict) {
            PebbleKit.sendAckToPebble(context, transaction_id);
            long data = dict.getUnsignedIntegerAsLong(1);
            switch(dict.getUnsignedIntegerAsLong(0).intValue()){
                case 0:
                    checkIfBooked();
                    break;
                case 1 :
                    storeKM(data);
                    break;
                case 2 :
                    storeSD(data);
                    break;
                case 3 :
                    storeST(data);
                    break;
                case 4 :
                    storeED(data);
                    break;
                case 5 :
                    storeET(data);
                    break;
                case 6 :
                    drivingT.setText("Status: unlocked");
                    break;
                case 7 :
                    checkOut();
                    break;
            }

            // A new AppMessage was received, tell Pebble
        }

    };
    public void checkIfBooked(){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        boolean isIt = false;
        Date it = null;
        for (int i = 0; i<bookingsSD.size();i++){
            if(bookingsSD.get(i).getTime().getTime() < new Date().getTime()) {
                isIt = true;
                it = bookingsED.get(i).getTime();
                break;
            }
        }
        if(isIt){
            sendMessageToPebble((byte) 0, it.getTime());
        }else{
            sendMessageToPebble((byte)0,0);
        }
    }
    public void storeKM(long data){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try {
            switch ((int)data){
                case 0:
                    tempKm = "10-20";
                    break;
                case 1:
                    tempKm ="20-50";
                    break;
                case 2:
                    tempKm ="50+";
                    break;
            }
            sendMessageToPebble((byte)1,(byte)1);
            Log.d("dsfd", "fdsfs");
        }catch (Exception e){
            sendMessageToPebble((byte) 1, (byte) 0);
        }
    }
    public void storeSD(long data){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        Calendar cal = Calendar.getInstance();
        cal.add(Calendar.DATE, (int) data);
        tempSD=cal;
        sendMessageToPebble((byte)2,(byte)1);
    }
    public void storeST(long data){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        tempSD.set(Calendar.HOUR_OF_DAY, (int) data / 100);
        tempSD.set(Calendar.MINUTE, (int) data%100);
        sendMessageToPebble((byte)3,(byte)1);
    }
    public void storeED(long data){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        Calendar cal = Calendar.getInstance();
        cal.add(Calendar.DATE, (int) data);
        tempED=cal;
        sendMessageToPebble((byte) 4, (byte) 1);
    }
    public void storeET(long data){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        tempED.set(Calendar.HOUR_OF_DAY, (int) data / 100);
        tempED.set(Calendar.MINUTE, (int) data%100);
        bookingsKm.add(tempKm);
        bookingsSD.add(tempSD);
        bookingsED.add(tempED);
        renderTables();
        sendMessageToPebble((byte) 5, 1);
    }
    public void checkOut(){
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        if(doorsOpen){
            sendMessageToPebble((byte) 6, 0);
        }else if(!keyIn){
            sendMessageToPebble((byte) 6, 1);
        }else if(!charging){
            sendMessageToPebble((byte) 6, 2);
        }else if(!geoFence){
            sendMessageToPebble((byte) 6, 3);
        }else{
            driving = false;
            drivingT.setText("Status: locked");
            sendMessageToPebble((byte) 6, score);
        }
    }
    public void renderTables(){
        for(int i = 0; i<bookingsKm.size();i++){
            SimpleDateFormat format1 = new SimpleDateFormat("hh:mm dd/MM");
            TextView kmT = new TextView(getApplicationContext());
            kmT.setText(bookingsKm.get(i));
            kmT.setGravity(Gravity.CENTER_HORIZONTAL);
            kmT.setTextSize(15);
            kmT.setTextColor(0xff000000);
            bKML.addView(kmT);

            TextView sdT = new TextView(getApplicationContext());
            sdT.setText(format1.format(bookingsSD.get(i).getTime()));
            sdT.setGravity(Gravity.CENTER_HORIZONTAL);
            sdT.setTextSize(15);
            sdT.setTextColor(0xff000000);
            bSDL.addView(sdT);

            TextView edT = new TextView(getApplicationContext());
            edT.setText(format1.format(bookingsED.get(i).getTime()));
            edT.setGravity(Gravity.CENTER_HORIZONTAL);
            edT.setTextSize(15);
            edT.setTextColor(0xff000000);
            bEDL.addView(edT);
        }
    }
}
