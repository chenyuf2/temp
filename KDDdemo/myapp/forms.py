from django import forms

class ContactForm(forms.Form):
    name = forms.CharField(label='', widget=forms.TextInput(attrs={'class':'col-lg-7', 'placeholder':'Please type in category names(words or phrases) separated by comma.','autocomplete': 'off'}))
    # email  = forms.EmailField(label='E-mail')
