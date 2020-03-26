function res = ph_rot(az,el,L)
res = zeros(1,length(az));
for i = 1:length(az)
    rot = roty(el(i))*rotz(-az(i))*[L;0;0];
    res(i) = rot(1);
end