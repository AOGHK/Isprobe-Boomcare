package com.physi.dev.nursinglight.list;

import android.annotation.SuppressLint;
import android.bluetooth.BluetoothDevice;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.physi.dev.nursinglight.R;

import java.util.LinkedList;
import java.util.List;

public class BleAdapter extends RecyclerView.Adapter<BleHolder> {

    public interface OnItemListener{
        void onSelectedDevice(BluetoothDevice device);
    }

    private OnItemListener listener;

    public void setOnItemListener(OnItemListener listener){
        this.listener = listener;
    }

    private List<BluetoothDevice> devices = new LinkedList<>();

    @NonNull
    @Override
    public BleHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View view = inflater.inflate(R.layout.view_ble_item, parent, false);
        return new BleHolder(view);
    }

    @SuppressLint("MissingPermission")
    @Override
    public void onBindViewHolder(@NonNull BleHolder holder, int position) {
        BluetoothDevice device = devices.get(position);

        holder.tvName.setText(device.getName());
        holder.tvAddress.setText(device.getAddress());

        holder.item.setOnClickListener(v -> {
            if(listener != null)
                listener.onSelectedDevice(device);
        });
    }

    @Override
    public int getItemCount() {
        return devices.size();
    }

    @SuppressLint("NotifyDataSetChanged")
    public void clearItems(){
        devices.clear();
        notifyDataSetChanged();
    }

    @SuppressLint({"MissingPermission", "NotifyDataSetChanged"})
    public void addItem(BluetoothDevice device){
        if (device.getName() == null || device.getName().isEmpty()){
            return;
        }
        if (devices.contains(device)){
            return;
        }

        devices.add(device);
        notifyDataSetChanged();
    }
}
